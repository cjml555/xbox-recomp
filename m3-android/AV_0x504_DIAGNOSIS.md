# Diagnóstico: access violation guest 0x504 (2026-11-20)

## Resumen
- El stub `MCLA_StreamBufferProbe` **NO es la causa** del bucle de access violations.
- El fault real es un **AddRef atómico sobre un puntero basura 0x500** (guest `sub_82422160`).
- Causa raíz más probable (respaldada por referencias): **m3-android nunca configuró
  `config.audio_factory` ni `config.input_factory`** → el juego aborta el init de su
  motor de audio y luego dereferencia el motor que nunca creó (patrón documentado
  textualmente en LARecomp y holdmysocks/mcla-recomp).
- Datos: `xarchive_cache.rpf` y `xarchive_audlo.rpf` están COMPLETOS; el ISO truncado
  (5.10 GB de 6.57 GB) solo afecta audio/music/bik/nxeart.

## Evidencia real (dispositivo ZY22KLVX6P, build con crash-trace v1)
Logcat tras tap JUGAR (19:41, 43,713 AVs en 30 s):
```
E MCLA_CrashTrace: AV #2200000 read guest 0x00000504 | host pc 0x79bd9f9130 = sub_82422160+0xDC | libmcla_main.so+0x13D7130
```
Desensamblado de `libmcla_main.so+0x13D7130` (llvm-objdump, build sin strip):
```
13d712c: add x2, x20, x8      ; x20 = guest membase, x8 = ctx.r11.u32
13d7130: ldr w22, [x2]        ; <-- fault: lwarx r10,0,r11  (r11 = r3+4)
```
→ guest `r3 = 0x500`, `lwarx [0x504]`. `sub_82422160` es un incremento atómico de
refcount (lwarx/stwcx sobre [r3+4], recursión si count==1). Algún caller le pasa
0x500 (puntero nulo + offset o valor basura). El handler del SDK devuelve false y
la instrucción se reintenta → bucle infinito (~12k AVs/s).

## Por qué el stub StreamBufferProbe no es culpable
1. Semántica codegen: `[[midasm_hook]]` se emite ANTES de la instrucción original y
   la instrucción original SE EJECUTA igual (`function_graph.cpp:576-598`,
   `builders/context.cpp:347`). El hook no reemplaza la función ni su retorno.
2. Referencia real (LARecomp `src/mc_engine/hooks/diagnostics.cpp:718`): el probe es
   observador puro, gateado por cvar `fxc_probe` (off por defecto); con cvar off hace
   `return` inmediato sin tocar registros. Nuestro stub (solo SDL_Log) era
   funcionalmente equivalente; el spam de logs era ruido, no la causa.
3. Cambio aplicado: stub silenciado (fiel al comportamiento default de la referencia).

## Fixes implementados (basados en referencias, build OK)
1. **Audio con fallback silencioso** — `app/jni/src/audio_output.h` (adaptación de
   LARecomp `src/mc_engine/music/audio_output.h`, sin capture toolkit):
   `OutputWithFallback : SDLAudioSystem` + `NullOutputDriver` que consume frames a
   256 samples/48 kHz y libera el semáforo. Wired: `config.audio_factory`.
   - LARecomp `larecomp_app.h:460`: `config.audio_factory = &mcla::audio::CreateAudioSystem;`
   - holdmysocks `src/mcla_app.cpp`: `config.audio_factory = &CreateMclaAudioSystem;`
   - holdmysocks `src/audio_fallback.cpp` (comment): "The game aborts its audio engine
     init if the host cannot create an audio driver, and later dereferences the engine
     it never created."
   - LARecomp `audio_output.h` (comment): "The game gives up on its whole audio engine
     when the host cannot open an output device, then dereferences that engine later
     and crashes."
   - El `NopAudioSystem` del SDK NO sirve: `CreateDriver` devuelve
     `X_STATUS_NOT_IMPLEMENTED` (src/audio/nop/nop_audio_system.cpp).
2. **Input system** — `config.input_factory = CreateDefaultInputSystem + SharedAssignment`
   (LARecomp `larecomp_app.h:466-470`; todos los pads = player 1, si no el juego dice
   "no signed in" y no guarda).
3. **Unidad t:** — `fs->RegisterSymbolicLink("t:", "\\Device\\Harddisk0\\Partition1")`
   tras Setup (LARecomp `larecomp_app.h:1200-1205`, holdmysocks `mcla_app.cpp`
   OnPostSetup). El juego sondea datos sueltos de ciudad en t:.
4. **Crash-trace v2** — `app/jni/src/crash_trace.cpp`: además del mapeo host PC→guest
   function (patrón de holdmysocks `src/crash_trace.cpp`), recupera el PPCContext desde
   x19..x28 y loguea el guest LR (= caller exacto) en los primeros 12 AVs.

## Auditoría de datos (extract-xiso -l MCLA.iso vs archivos extraídos)
ISO declara 6,569,586,392 bytes; el archivo tiene 5,102,370,816 (faltan ~1.47 GB al final).
| Archivo | Declarado | Extraído | Estado |
|---|---|---|---|
| default.xex | 9,252,864 | 9,252,864 | OK |
| xarchive_cache.rpf | 2,130,739,200 | 2,130,739,200 | **COMPLETO** |
| xarchive_audlo.rpf | 1,463,189,504 | 1,463,189,504 | **COMPLETO** |
| xarchive_audio.rpf | 1,615,757,312 | 1,182,990,336 | truncado (-432 MB) |
| xarchive_music.rpf | 780,304,384 | 0 | falta |
| *.bik (6) | 63-123 MB c/u | 0 | faltan |
| nxeart | 1,425,408 | 0 | falta |

## Estado del test
- Build final: `app-debug.apk` (21:13) con los 4 cambios — **BUILD SUCCESSFUL**.
- **INSTALL+TEST BLOQUEADO**: el teléfono se desconectó físicamente del USB (~19:52,
  no aparece en lsusb); WiFi adb rechazado (tcpip off, sin wireless debugging mDNS).
  Se requiere reconectar el cable USB. Watcher activo esperando al dispositivo.
- El crash-trace v1 SÍ se probó en dispositivo (evidencia de arriba). El v2 + fixes
  quedan pendientes de install/test en cuanto el dispositivo vuelva.

## Próximo paso exacto (al reconectar)
```
adb install -r m3-android/app/build/outputs/apk/debug/app-debug.apk
adb shell am force-stop com.mcla.recomp && adb logcat -G 16M && adb logcat -c
adb shell am start -n com.mcla.recomp/.MainActivity
# esperar menú (~15 s), tap JUGAR (1222,465), logcat 30 s
adb logcat -d -s MCLA_CrashTrace MCLA_JNI spdlog
```
- Si el bucle cesó → el AV era código (audio engine null), no datos.
- Si persiste → el crash-trace v2 da el caller exacto (guest LR) para el siguiente fix;
  y/o se confirma que hace falta el dump completo (audio.rpf/music.rpf).
