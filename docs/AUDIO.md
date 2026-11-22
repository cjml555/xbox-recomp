# Audio Output Chain Architecture

This document describes the audio output system used by the MCLA Android port. The audio chain is implemented in `m3-android/app/jni/src/audio_output.h` and follows a priority-based fallback pattern to ensure audio works across the widest range of Android devices and API levels.

## Priority Order

The audio system attempts to initialize drivers in this strict order:

| Priority | Driver | Description |
|----------|--------|-------------|
| 1 | **AAudioDriver** | Native Android AAudio API (API 26+), lowest latency, loaded via `dlopen` |
| 2 | **SDLAudioSystem** | Xenia's SDL3 audio backend (fallback when AAudio unavailable or fails) |
| 3 | **NullOutputDriver** | Silent driver that consumes frames to keep guest timing alive |

The first driver to successfully initialize becomes the active audio output. The game **requires** a working driver — it aborts its audio engine initialization if no driver can be created, and later dereferences the uninitialized engine pointer. The `NullOutputDriver` exists solely as a last-resort timing keeper so the guest audio worker thread doesn't deadlock.

## OutputWithFallback Pattern

`OutputWithFallback` (derived from `SDLAudioSystem`) implements the chained fallback logic in its `CreateDriver()` override:

```cpp
X_STATUS CreateDriver(size_t index, Semaphore* semaphore, AudioDriver** out_driver) override {
    // Tier 1: AAudio (native, preferred on API 26+)
    if (AAudioDriver::IsAvailable()) {
        auto* aaudio = new AAudioDriver(memory_, semaphore);
        if (aaudio->Initialize()) {
            *out_driver = aaudio;
            return X_STATUS_SUCCESS;
        }
        delete aaudio;  // falls through to SDL
    }

    // Tier 2: SDL (Xenia's backend)
    if (SDLAudioSystem::CreateDriver(index, semaphore, out_driver) == X_STATUS_SUCCESS) {
        return X_STATUS_SUCCESS;
    }

    // Tier 3: Null (silent but keeps guest timing)
    *out_driver = new NullOutputDriver(memory_, semaphore);
    return X_STATUS_SUCCESS;
}
```

Each tier is tried sequentially. AAudio is checked for availability via `dlopen("libaaudio.so")` so the app remains loadable on API < 26 where the library doesn't exist. If AAudio loads but `Initialize()` fails (e.g., device rejects stream parameters), it falls through to SDL. SDL uses the standard Xenia `SDLAudioSystem` path. Only when both fail does the `NullOutputDriver` activate.

## Driver Details

### AAudioDriver (Priority 1)

**File:** `audio_output.h` lines 129–232

- **Architecture:** Dynamically loads AAudio symbols via `dlopen("libaaudio.so")` at runtime. This allows the build target to remain at API 24 while using AAudio when present (API 26+).
- **Stream format:** 48 kHz, stereo (2 channels), 32-bit float (`AAUDIO_FORMAT_PCM_FLOAT`), shared mode.
- **Callback:** Runs on a dedicated high-priority AAudio callback thread. Must not block — pops the next queued guest frame (or silence) and returns.
- **Frame handling:** Guest produces 6-channel 48 kHz frames (256 samples). Driver downmixes to stereo float via `conversion::sequential_6_BE_to_interleaved_2_LE` with `StereoFold` fold-down.
- **Buffer sizing:** Requests `capacity / 2` frames (clamped by AAudio to burst size multiples) to minimize latency.
- **Performance mode:** **Does not request `AAUDIO_PERFORMANCE_MODE_LOW_LATENCY`** — see Device Quirks below.

### SDLAudioSystem (Priority 2)

**File:** `rex/audio/sdl/sdl_audio_system.h` (Xenia upstream)

- Standard SDL3 audio backend from Xenia. Opens an SDL audio device with the requested format.
- Used when AAudio is unavailable (API < 26) or fails to initialize.
- Format negotiation handled by SDL; typically stereo float or int16.

### NullOutputDriver (Priority 3)

**File:** `audio_output.h` lines 237–283

- **Purpose:** Silent output that keeps the guest audio worker's timing alive.
- **Mechanism:** A background thread (`clock_`) wakes every **5.333 ms** (256 samples @ 48 kHz = 256,000,000 ns / 48,000 ≈ 5.33 ms) and releases the frame semaphore.
- **Guest frame consumption:** `SubmitFrame()` increments a counter; the clock thread decrements it each period and releases the semaphore. If frames back up, they drain at the hardware rate.
- **No audio output:** Writes zeros to any hypothetical buffer; the guest hears silence but continues running.

## Device Quirks Handling

### Motorola Edge 50 Fusion (and similar devices)

The Motorola Edge 50 Fusion (Snapdragon 7s Gen 2) exhibits a specific AAudio quirk documented in the self-test logic (`m3-android/app/jni/src/jni_bridge.cpp`, lines 360–378):

1. **`AAUDIO_DIRECTION_OUTPUT` is `0`** — not `1` (which is `AAUDIO_DIRECTION_INPUT` = record path, would trigger `RECORD_AUDIO` permission denial).
2. **`AAUDIO_FORMAT_PCM_FLOAT` is `2`** — not `1` (`AAUDIO_FORMAT_PCM_I16`).
3. **`AAUDIO_PERFORMANCE_MODE_LOW_LATENCY` is rejected** with `AAUDIO_ERROR_ILLEGAL_ARGUMENT` on this device. The driver **leaves the default performance mode (`NONE`)** so the stream opens successfully.

The self-test (`AAUDIO_SELFTEST`) runs at startup (in `jni_bridge.cpp` `OnPostSetup`) to verify AAudio works on the current device before the game initializes its audio engine. If the probe fails, the system gracefully falls back to SDL.

**Log output example:**
```
I/MCLA_JNI_AUDIO: AAUDIO_SELFTEST OK: stream opened at 48 kHz stereo float
```
or on failure:
```
W/MCLA_JNI_AUDIO: AAUDIO_SELFTEST FAILED: AAudio unavailable — will fall back to SDL/null
```

### General AAudio Compatibility Notes

| Issue | Handling |
|-------|----------|
| API < 26 (no `libaaudio.so`) | `dlopen` fails → `IsAvailable()` returns `false` → skip to SDL |
| Missing symbols | `dlsym` failure → `Load()` returns `false` → skip to SDL |
| Stream open fails | `Initialize()` returns `false` → delete driver → skip to SDL |
| Low-latency mode rejected | Code explicitly avoids `LOW_LATENCY`; uses default `NONE` |
| Direction/format constants wrong | Hardcoded correct enum values in `audio_output.h` (not from headers) |

## Adding a New Driver

To insert a new audio driver into the fallback chain:

### 1. Implement `AudioDriver` Interface

Create a class deriving from `rex::audio::AudioDriver` (from ReXGlue SDK):

```cpp
class MyAudioDriver final : public rex::audio::AudioDriver {
public:
    MyAudioDriver(rex::memory::Memory* memory, rex::thread::Semaphore* semaphore)
        : AudioDriver(memory), semaphore_(semaphore) {}

    ~MyAudioDriver() override { Shutdown(); }

    static bool IsAvailable() { /* runtime probe */ }

    bool Initialize() { /* open device, start callback */ }

    void SubmitFrame(uint32_t frame_ptr) override {
        // Convert guest 6-ch BE float frame to your output format
        // Queue for callback thread
    }

    void Shutdown() { /* stop device, cleanup */ }

private:
    // Callback runs on audio thread — must be non-blocking
    static void Callback(void* userdata, void* buffer, int frames) { ... }

    rex::thread::Semaphore* semaphore_;
    // your state...
};
```

**Key requirements:**
- `SubmitFrame()` receives a **guest virtual pointer** to a 6-channel big-endian float frame (256 samples @ 48 kHz). Use `memory_->TranslateVirtual<float*>(frame_ptr)` to get a host pointer.
- Convert using `rex::audio::conversion` utilities (e.g., `sequential_6_BE_to_interleaved_2_LE` for stereo fold-down).
- Release `semaphore_` **once per frame consumed** so the guest audio worker advances.
- Callback must be **real-time safe**: no locks held across audio API calls, no allocations, no blocking.

### 2. Add to `OutputWithFallback::CreateDriver()`

Insert your driver at the desired priority in the fallback chain (edit `audio_output.h` lines 295–323):

```cpp
// Tier N: Your new driver (insert at desired priority)
if (MyAudioDriver::IsAvailable()) {
    auto* my_driver = new MyAudioDriver(memory_, semaphore);
    if (my_driver->Initialize()) {
        LOGI("[audio] MyAudioDriver active");
        *out_driver = my_driver;
        return X_STATUS_SUCCESS;
    }
    LOGW("[audio] MyAudioDriver init failed, falling through");
    delete my_driver;
}
```

### 3. Handle Destruction in `DestroyDriver()`

Add a branch to `DestroyDriver()` (lines 325–336) to clean up your driver type:

```cpp
void DestroyDriver(AudioDriver* driver) override {
    if (dynamic_cast<MyAudioDriver*>(driver)) {
        LOGI("[audio] destroying MyAudioDriver");
        delete driver;
    } else if (dynamic_cast<AAudioDriver*>(driver)) {
        ...
    }
}
```

### 4. Verify Build and Test

- Ensure the new driver compiles for the Android target (ARM64, API 24+).
- Test on devices with and without the new backend available.
- Verify fallback behavior: disable higher-priority drivers and confirm your driver activates.
- Check logcat for `[audio]` tags to confirm the expected driver is selected.

## Related Files

| File | Purpose |
|------|---------|
| `m3-android/app/jni/src/audio_output.h` | Complete audio chain implementation |
| `m3-android/app/jni/src/jni_bridge.cpp` | AAudio self-test at startup (lines 360–378) |
| `rex/audio/audio_driver.h` | Base `AudioDriver` interface (ReXGlue SDK) |
| `rex/audio/sdl/sdl_audio_system.h` | SDL3 backend (Xenia upstream) |
| `rex/audio/conversion.h` | Guest frame format conversion utilities |

## Quick Reference: Frame Timing

| Parameter | Value |
|-----------|-------|
| Sample rate | 48,000 Hz |
| Frame size | 256 samples |
| Frame period | 5.333 ms (256,000,000 ns / 48,000) |
| Guest format | 6 channels, big-endian, float32 |
| AAudio output | 2 channels (stereo), float32, little-endian |
| SDL output | Negotiated (typically stereo float or int16) |

---

*See also: [AAUDIO_SELFTEST.md](../AAUDIO_SELFTEST.md) — Motorola Edge 50 Fusion AAudio behavior notes and self-test procedure.*