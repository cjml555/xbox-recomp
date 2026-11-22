# MCLA AAudio Self-Test & Driver Fixes

Diagnostic notes for the AAudio output driver on the Android port of the
MCLA (Midtown Crazy Adventures) recomp. Covers what the self-test does, why it
is diagnostic-only, and the three device-specific bugs it was written to catch.

- Self-test source: `app/jni/src/jni_bridge.cpp` (lines ~355–378)
- Driver source: `app/jni/src/audio_output.h`
- Log tag: `MCLA_JNI_AUDIO`; self-test line prefix: `AAUDIO_SELFTEST`

---

## 1. What the self-test does

The guest (Xbox 360) game aborts its audio engine initialization if the host
cannot create an output driver, and later dereferences the engine it never
created — so a working driver is mandatory. The self-test runs once, after
`Runtime::Setup` succeeds and before the game thread requests audio for real:

1. Confirms a function dispatcher and guest memory are available.
2. Creates a counting semaphore (`Semaphore::Create(0, 1)`).
3. Constructs an `mcla::audio::AAudioDriver` with that memory and semaphore.
4. Calls `probe.Initialize()`, which:
   - `dlopen`s `libaaudio.so` (the app build target stays at API 24; AAudio,
     API 26+, is only used when present).
   - Builds and opens a **48 kHz, stereo, floating-point** output stream
     (2 channels, float format, shared mode, default performance mode).
   - Starts the stream and requests a half-capacity buffer for lower latency.
5. Logs the outcome:
   - `AAUDIO_SELFTEST OK: stream opened at 48 kHz stereo float` then
     `probe.Shutdown()` (stop + close the stream).
   - `AAUDIO_SELFTEST FAILED: ...` on any failure.
   - `AAUDIO_SELFTEST SKIPPED: ...` if no memory or semaphore could be created.

## 2. Why it is diagnostic-only

The self-test is a **probe, not a fix**. It verifies the device's AAudio
configuration is usable before the guest relies on it, and surfaces the result
in logcat under the `AAUDIO_SELFTEST` filter so device-specific breakage is
caught early. Key properties:

- It **never blocks boot**. Every failure path just logs a warning and falls
  through to normal startup. There is no retry loop, sleep, or exit-on-error.
- It is **not the actual audio path**. Game audio still goes through the real
  `OutputWithFallback` chain (`AAudio → SDL → Null`) when gameplay begins. The
  probe only proves the AAudio stream can be opened on this device.
- If AAudio is broken, the real chain degrades gracefully to SDL, then to a
  silent null driver that keeps the guest audio worker's timing (256 samples @
  48 kHz) — so the port still runs, just without native low-latency audio.
- The `Initialize()` success/failure result is discarded; the driver is shut
  down immediately and a fresh one is created by the real chain later.

---

## 3. The three device-specific bugs fixed

Found on a **Motorola Edge 50 Fusion running Android 16**. All three are in
`audio_output.h`; each caused AAudio to reject the stream with
`AAUDIO_ERROR_*` and fall through to SDL/null audio.

### 3.1 `kAaudioDirectionOutput` = 1 → 0

**Bug:** was `1`.

**Fix:** `0` — `AAUDIO_DIRECTION_OUTPUT`.

**Why:** the AAudio direction enum is `AAUDIO_DIRECTION_OUTPUT = 0` and
`AAUDIO_DIRECTION_INPUT = 1`. The old value `1` asked for an **input**
(capture) stream, which maps to `AAudioStreamBuilder_createRecord()`. On most
devices an input stream without a microphone permission grant fails with
`AAUDIO_ERROR_ACCESS_DENIED` — so no stream could ever open.

### 3.2 `kAaudioFormatFloat` = 1 → 2

**Bug:** was `1`.

**Fix:** `2` — `AAUDIO_FORMAT_PCM_FLOAT`.

**Why:** the format enum is `AAUDIO_FORMAT_PCM_I16 = 1` and
`AAUDIO_FORMAT_PCM_FLOAT = 2`. The old value `1` requested 16-bit integer
samples, but the driver downmix path and `DataCallback` write **interleaved
32-bit `float`** samples. An int16 stream fed with float data would produce
garbage/overflow. The constant now matches the actual float pipeline.

### 3.3 Removed `AAUDIO_PERFORMANCE_MODE_LOW_LATENCY`

**Bug:** the builder called
`setPerformanceMode(kAaudioPerfLowLatency /* 2 */)`.

**Fix:** the call is removed; the stream uses the default
`AAUDIO_PERFORMANCE_MODE_NONE`.

**Why:** this Motorola device (and some other OEM builds) rejects
`LOW_LATENCY` with `AAUDIO_ERROR_ILLEGAL_ARGUMENT` because the
fastest/`LOW_LATENCY` performance mode requires an uncompressed, non-tunneled,
non-offload stream configuration that the device's mixer does not expose for
the requested format. Removing the call lets the stream open under the default
mode; buffer sizing still keeps latency low.

---

## 4. Build & run

```sh
# Build (JDK pinned)
export JAVA_HOME=/home/jhona/tools/jdk-21.0.5+11
cd m3-android && ./gradlew assembleDebug --no-daemon
```

```sh
# Install + capture the self-test result
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb logcat -s AAUDIO_SELFTEST      # also MCLA_JNI_AUDIO for the surrounding chain
```

Launch the app; you should see one `AAUDIO_SELFTEST OK ...` line in logcat.

---

## 5. Related audio-chain reference

Audio output priority in `audio_output.h`:

1. **AAudioDriver** — native, low-latency Android audio (dlopen'd, API 26+).
2. **SDLAudioSystem** — Xenia's SDL3 backend (falls through if no device).
3. **NullOutputDriver** — silent, but consumes frames at the hardware rate so
   the guest audio worker keeps timing (256 samples @ 48 kHz).

The fallback chain lives in `OutputWithFallback::CreateDriver`
(`audio_output.h`, ~lines 295–323).