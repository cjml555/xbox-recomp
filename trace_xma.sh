#!/usr/bin/env bash
# Tight repro for the MCLA APU/XMA trace. Run with the device attached via USB.
set -euo pipefail

ROOT=/home/jhona/xbox360-recomp
APK="$ROOT/m3-android/app/build/outputs/apk/debug/app-debug.apk"
OUT="${1:-$ROOT/m3-android/xma_trace_$(date +%Y%m%d_%H%M%S).log}"

echo "[trace] waiting for device..."
adb wait-for-device
adb root >/dev/null 2>&1 || true
adb shell am force-stop com.mcla.recomp || true
adb logcat -c
adb install -r "$APK"
adb shell am start -n com.mcla.recomp/.MainActivity

echo "[trace] waiting 20 s for the menu..."
sleep 20

# Menu coord from the previous device session; adjust if the layout changed.
adb shell input tap 2050 449

echo "[trace] capturing 35 s after JUGAR..."
sleep 35
adb logcat -d -v threadtime \
  -s MCLA_AudioTrace:I MCLA_VoiceTrace:I MCLA_XMA:I MCLA_XMA_DIAG:I \
     MCLA_JNI:I MCLA_JNI_AUDIO:I MCLA_MMIO:I MCLA_CrashTrace:E spdlog:I \
  > "$OUT"

echo "[trace] log: $OUT"
echo "--- key evidence ---"
grep -E 'XMA_READY|AUDIO_SETUP|XAudioRegisterRenderDriverClient|REGISTER_CLIENT|indirect #.*(voice_enable|xma_kick|create_context|mixer_thread|audio_callback)|vdisp #|ctx_setup #|KICK r=|MCLA_MMIO STORE 7FEA|WORKER allocated|AUDIO_WORKER alive pump' "$OUT" | head -120 || true

echo
echo "[trace] interpretation:"
echo "  XMA_READY + AUDIO_SETUP => decoder alive and APU MMIO range registered."
echo "  MCLA_MMIO STORE 7FEA... => guest is writing APU registers (kick/lock/clear)."
echo "  indirect target=0x823FB5B8 voice_enable => COM voice activation reached."
echo "  No voice_enable but vdisp target=0x... => follow that target in larecomp_init.cpp."
echo "  If pump stays 0, enter a race first: menus alone may not register an audio client."
