#pragma once

// Audio output chain for MCLA on Android, in priority order:
//   1. AAudioDriver  — native, low-latency Android audio (loaded via dlopen
//                      so the app still runs on API < 26 where AAudio is absent).
//   2. SDLAudioSystem — Xenia's SDL3 backend (falls through if no device opens).
//   3. NullOutputDriver — silent, but consumes frames at the hardware rate so the
//                      guest audio worker keeps its timing (256 samples @ 48 kHz).
//
// The game aborts its audio engine init if the host cannot create a driver, and
// later dereferences the engine it never created — so a usable driver is
// mandatory; hence the silent null tier as a last resort.

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <rex/audio/audio_driver.h>
#include <rex/audio/conversion.h>
#include <rex/audio/sdl/sdl_audio_system.h>
#include <rex/logging.h>
#include <rex/thread.h>

#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "MCLA_JNI_AUDIO"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace mcla::audio {

using rex::X_STATUS;

// XAudio hands out 256-sample frames at 48 kHz: 5.333 ms each.
inline constexpr std::chrono::nanoseconds kNullFramePeriod{256'000'000'000ll / 48'000};

// ─────────────────────────────────────────────────────────────────────────────
// Minimal AAudio binding loaded dynamically with dlopen so the build target can
// stay at API 24 while AAudio (API 26+) is used when present.
// ─────────────────────────────────────────────────────────────────────────────
struct AAudioApi {
  void* handle = nullptr;

  int (*StreamBuilder_create)(void** builder) = nullptr;
  void (*StreamBuilder_delete)(void* builder) = nullptr;
  void (*StreamBuilder_setDirection)(void* builder, int32_t direction) = nullptr;
  void (*StreamBuilder_setChannelCount)(void* builder, int32_t count) = nullptr;
  void (*StreamBuilder_setSampleRate)(void* builder, int32_t rate) = nullptr;
  void (*StreamBuilder_setFormat)(void* builder, int32_t format) = nullptr;
  void (*StreamBuilder_setSharingMode)(void* builder, int32_t mode) = nullptr;
  void (*StreamBuilder_setPerformanceMode)(void* builder, int32_t mode) = nullptr;
  void (*StreamBuilder_setDataCallback)(void* builder, void* callback, void* userdata) = nullptr;
  int (*StreamBuilder_openStream)(void* builder, void** stream) = nullptr;
  int32_t (*Stream_getFramesPerBurst)(void* stream) = nullptr;
  int32_t (*Stream_getChannelCount)(void* stream) = nullptr;
  int32_t (*Stream_getSampleRate)(void* stream) = nullptr;
  int (*Stream_setBufferSizeInFrames)(void* stream, int32_t frames) = nullptr;
  int32_t (*Stream_getBufferCapacityInFrames)(void* stream) = nullptr;
  int (*Stream_requestStart)(void* stream) = nullptr;
  int (*Stream_requestStop)(void* stream) = nullptr;
  int (*Stream_close)(void* stream) = nullptr;

  bool Load() {
    if (handle) return true;
    handle = dlopen("libaaudio.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
      LOGW("[audio] libaaudio.so not available: %s", dlerror());
      return false;
    }
    auto sym = [&](const char* name, auto* out) -> bool {
      *reinterpret_cast<void**>(out) = dlsym(handle, name);
      if (!*reinterpret_cast<void**>(out)) {
        LOGE("[audio] missing AAudio symbol %s", name);
        return false;
      }
      return true;
    };
    bool ok = true;
    ok &= sym("AAudio_createStreamBuilder", &StreamBuilder_create);
    ok &= sym("AAudioStreamBuilder_delete", &StreamBuilder_delete);
    ok &= sym("AAudioStreamBuilder_setDirection", &StreamBuilder_setDirection);
    ok &= sym("AAudioStreamBuilder_setChannelCount", &StreamBuilder_setChannelCount);
    ok &= sym("AAudioStreamBuilder_setSampleRate", &StreamBuilder_setSampleRate);
    ok &= sym("AAudioStreamBuilder_setFormat", &StreamBuilder_setFormat);
    ok &= sym("AAudioStreamBuilder_setSharingMode", &StreamBuilder_setSharingMode);
    ok &= sym("AAudioStreamBuilder_setPerformanceMode", &StreamBuilder_setPerformanceMode);
    ok &= sym("AAudioStreamBuilder_setDataCallback", &StreamBuilder_setDataCallback);
    ok &= sym("AAudioStreamBuilder_openStream", &StreamBuilder_openStream);
    ok &= sym("AAudioStream_getFramesPerBurst", &Stream_getFramesPerBurst);
    ok &= sym("AAudioStream_getChannelCount", &Stream_getChannelCount);
    ok &= sym("AAudioStream_getSampleRate", &Stream_getSampleRate);
    ok &= sym("AAudioStream_setBufferSizeInFrames", &Stream_setBufferSizeInFrames);
    ok &= sym("AAudioStream_getBufferCapacityInFrames", &Stream_getBufferCapacityInFrames);
    ok &= sym("AAudioStream_requestStart", &Stream_requestStart);
    ok &= sym("AAudioStream_requestStop", &Stream_requestStop);
    ok &= sym("AAudioStream_close", &Stream_close);
    if (!ok) {
      dlclose(handle);
      handle = nullptr;
      return false;
    }
    return true;
  }
};

inline AAudioApi& GetAAudio() {
  static AAudioApi api;
  return api;
}

// AAudio stream layout. We open a stereo float stream and downmix the guest's
// 6-channel 48 kHz frames. Constant AAUDIO enums (not included to stay header-free).
inline constexpr int32_t kAaudioDirectionOutput = 0;  // AAUDIO_DIRECTION_OUTPUT (0), NOT 1 which is INPUT
inline constexpr int32_t kAaudioFormatFloat = 2;     // AAUDIO_FORMAT_PCM_FLOAT
inline constexpr int32_t kAaudioSharingShared = 0;    // AAUDIO_SHARING_MODE_SHARED
inline constexpr int32_t kAaudioPerfLowLatency = 2;   // AAUDIO_PERFORMANCE_MODE_LOW_LATENCY
// AAUDIO_CALLBACK_RESULT_* enums (not included to stay header-free)
inline constexpr int32_t kAaudioCallbackContinue = 0;  // AAUDIO_CALLBACK_RESULT_CONTINUE
inline constexpr int32_t kAaudioCallbackStop = 1;      // AAUDIO_CALLBACK_RESULT_STOP

inline constexpr int32_t kFrameFreq = 48000;  // 48 kHz
inline constexpr int32_t kChannelSamples = 256;

class AAudioDriver final : public rex::audio::AudioDriver {
 public:
  AAudioDriver(rex::memory::Memory* memory, rex::thread::Semaphore* semaphore)
      : AudioDriver(memory), semaphore_(semaphore) {}

  ~AAudioDriver() override { Shutdown(); }

  static bool IsAvailable() { return GetAAudio().Load(); }

  bool Initialize() {
    AAudioApi& A = GetAAudio();
    if (!A.Load()) return false;

    void* builder = nullptr;
    if (A.StreamBuilder_create(&builder) != 0 || !builder) {
      LOGE("[aaudio] createStreamBuilder failed");
      return false;
    }
    A.StreamBuilder_setDirection(builder, kAaudioDirectionOutput);
    A.StreamBuilder_setFormat(builder, kAaudioFormatFloat);
    A.StreamBuilder_setSampleRate(builder, kFrameFreq);
    A.StreamBuilder_setChannelCount(builder, 2);
    A.StreamBuilder_setSharingMode(builder, kAaudioSharingShared);
    A.StreamBuilder_setDataCallback(builder, reinterpret_cast<void*>(&AAudioDriver::DataCallback),
                                    this);

    void* stream = nullptr;
    if (A.StreamBuilder_openStream(builder, &stream) != 0 || !stream) {
      LOGE("[aaudio] openStream failed");
      A.StreamBuilder_delete(builder);
      return false;
    }
    A.StreamBuilder_delete(builder);
    stream_ = stream;

    int32_t burst = A.Stream_getFramesPerBurst(stream);
    if (burst > 0) {
      int32_t capacity = A.Stream_getBufferCapacityInFrames(stream);
      int32_t want = capacity > 0 ? capacity / 2 : burst * 4;
      A.Stream_setBufferSizeInFrames(stream, want);
    }

    if (A.Stream_requestStart(stream) != 0) {
      LOGE("[aaudio] requestStart failed");
      return false;
    }

    LOGI("[aaudio] opened successfully: 2ch, %d Hz, burst=%d", kFrameFreq, burst);
    return true;
  }

  void SubmitFrame(uint32_t frame_ptr) override {
    const auto input_frame = memory_->TranslateVirtual<float*>(frame_ptr);
    // Convert 6ch BE to interleaved stereo LE, mirroring the SDL driver: the
    // guest frame is copied as-is into a pooled buffer, conversion happens here.
    float* out = nullptr;
    {
      std::lock_guard lock(mutex_);
      if (unused_.empty()) {
        out = new float[2 * kChannelSamples];
      } else {
        out = unused_.back();
        unused_.pop_back();
      }
    }
    rex::audio::conversion::sequential_6_BE_to_interleaved_2_LE(
        out, input_frame, kChannelSamples, rex::audio::StereoFold{}, 1.0f);

    {
      std::lock_guard lock(mutex_);
      ++submitted_frames_;
      queued_.push_back(out);
      if (submitted_frames_ == 1 || submitted_frames_ % 960 == 0) {
        float src_peak = 0.0f, dst_peak = 0.0f;
        for (int i = 0; i < 6 * kChannelSamples; ++i) {
          float v = input_frame[i]; if (v < 0) v = -v; if (v > src_peak) src_peak = v;
        }
        for (int i = 0; i < 2 * kChannelSamples; ++i) {
          float v = out[i]; if (v < 0) v = -v; if (v > dst_peak) dst_peak = v;
        }
        LOGI("[aaudio] SubmitFrame #%u (queued=%zu) src_peak=%.4f dst_peak=%.4f",
             submitted_frames_, queued_.size(), src_peak, dst_peak);
      }
    }
    // NOTE: semaphore Release is done in DataCallback per consumed frame, exactly
    // like the SDL driver. SubmitFrame must NOT release it.
  }

  void Shutdown() {
    AAudioApi& A = GetAAudio();
    std::lock_guard lock(mutex_);
    if (stream_) {
      A.Stream_requestStop(stream_);
      A.Stream_close(stream_);
      stream_ = nullptr;
    }
    for (float* f : queued_) delete[] f;
    queued_.clear();
    for (float* f : unused_) delete[] f;
    unused_.clear();
  }

 private:
  // AAudio runs this on a dedicated high-priority callback thread. It must not
  // block. Mirror SDL: drain queued guest frames (each holds kChannelSamples
  // stereo frames), copy up to num_frames, zero-fill any remainder, and release
  // the guest semaphore once per guest frame consumed so the audio clock stays
  // at 48 kHz.
  static int32_t DataCallback(void* stream, void* userdata, void* audio_data, int32_t num_frames) {
    auto* self = static_cast<AAudioDriver*>(userdata);
    float* dst = static_cast<float*>(audio_data);
    std::lock_guard lock(self->mutex_);
    int32_t frames_left = num_frames;
    float peak = 0.0f;
    while (frames_left > 0 && !self->queued_.empty()) {
      float* src = self->queued_.front();
      self->queued_.pop_front();
      const int32_t n = frames_left < kChannelSamples ? frames_left : kChannelSamples;
      std::memcpy(dst, src, sizeof(float) * 2 * n);
      for (int i = 0; i < 2 * n; ++i) {
        float v = dst[i];
        if (v < 0) v = -v;
        if (v > peak) peak = v;
      }
      dst += 2 * n;
      frames_left -= n;
      self->unused_.push_back(src);
      self->semaphore_->Release(1, nullptr);
      ++self->callback_frames_;
    }
    if (frames_left > 0) {
      std::memset(dst, 0, sizeof(float) * 2 * frames_left);
    }
    ++self->callback_calls_;
    if ((self->callback_calls_ % 50) == 1) {
      LOGI("[aaudio] callback #%u frames=%u queued=%zu peak=%.4f", self->callback_calls_,
           self->callback_frames_, self->queued_.size(), peak);
    }
    return kAaudioCallbackContinue;
  }

  rex::thread::Semaphore* semaphore_;
  void* stream_ = nullptr;
  std::deque<float*> queued_;
  std::deque<float*> unused_;
  uint32_t submitted_frames_ = 0;
  uint64_t callback_calls_ = 0;
  uint64_t callback_frames_ = 0;
  std::mutex mutex_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Null output (silent timing keeper) — used only if AAudio and SDL both fail.
// ─────────────────────────────────────────────────────────────────────────────
class NullOutputDriver final : public rex::audio::AudioDriver {
 public:
  NullOutputDriver(rex::memory::Memory* memory, rex::thread::Semaphore* frame_semaphore)
      : AudioDriver(memory), frame_semaphore_(frame_semaphore), clock_([this] { Consume(); }) {}

  ~NullOutputDriver() override {
    {
      std::lock_guard lock(mutex_);
      stop_ = true;
    }
    wake_.notify_one();
    clock_.join();
  }

  void SubmitFrame(uint32_t) override {
    {
      std::lock_guard lock(mutex_);
      ++queued_;
    }
    wake_.notify_one();
  }

 private:
  void Consume() {
    using Clock = std::chrono::steady_clock;
    Clock::time_point due{};
    std::unique_lock lock(mutex_);
    for (;;) {
      wake_.wait(lock, [this] { return stop_ || queued_ > 0; });
      if (stop_) return;
      const auto now = Clock::now();
      due = (due + kNullFramePeriod < now) ? now + kNullFramePeriod : due + kNullFramePeriod;
      if (wake_.wait_until(lock, due, [this] { return stop_; })) return;
      --queued_;
      lock.unlock();
      frame_semaphore_->Release(1, nullptr);
      lock.lock();
    }
  }

  rex::thread::Semaphore* frame_semaphore_;
  std::mutex mutex_;
  std::condition_variable wake_;
  uint32_t queued_ = 0;
  bool stop_ = false;
  std::thread clock_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Chained output system: try AAudio → SDL → Null.
// ─────────────────────────────────────────────────────────────────────────────
class OutputWithFallback final : public rex::audio::sdl::SDLAudioSystem {
 public:
  explicit OutputWithFallback(rex::runtime::FunctionDispatcher* function_dispatcher)
      : SDLAudioSystem(function_dispatcher) {
    LOGI("[audio] OutputWithFallback constructed (AAudio -> SDL -> null)");
  }

  X_STATUS CreateDriver(size_t index, rex::thread::Semaphore* semaphore,
                        rex::audio::AudioDriver** out_driver) override {
    LOGI("[audio] CreateDriver requested (index=%zu)", index);

    // Tier 1: AAudio (native, preferred on API 26+).
    if (AAudioDriver::IsAvailable()) {
      auto* aaudio = new AAudioDriver(memory_, semaphore);
      if (aaudio->Initialize()) {
        LOGI("[audio] AAudio driver active");
        *out_driver = aaudio;
        return X_STATUS_SUCCESS;
      }
      LOGW("[audio] AAudio init failed, falling through to SDL");
      delete aaudio;
    } else {
      LOGI("[audio] AAudio unavailable, using SDL");
    }

    // Tier 2: SDL (Xenia's backend).
    if (SDLAudioSystem::CreateDriver(index, semaphore, out_driver) == X_STATUS_SUCCESS) {
      LOGI("[audio] SDL audio driver opened successfully");
      return X_STATUS_SUCCESS;
    }

    // Tier 3: Null (silent but keeps guest timing).
    LOGW("[audio] no output device could be opened; continuing without sound");
    *out_driver = new NullOutputDriver(memory_, semaphore);
    return X_STATUS_SUCCESS;
  }

  void DestroyDriver(rex::audio::AudioDriver* driver) override {
    if (dynamic_cast<AAudioDriver*>(driver)) {
      LOGI("[audio] destroying AAudioDriver");
      delete driver;
    } else if (auto* null_driver = dynamic_cast<NullOutputDriver*>(driver)) {
      LOGI("[audio] destroying NullOutputDriver");
      delete null_driver;
    } else {
      LOGI("[audio] destroying SDL audio driver");
      SDLAudioSystem::DestroyDriver(driver);
    }
  }
};

inline std::unique_ptr<rex::system::IAudioSystem> CreateAudioSystem(
    rex::runtime::FunctionDispatcher* function_dispatcher) {
  return std::make_unique<OutputWithFallback>(function_dispatcher);
}

}  // namespace mcla::audio