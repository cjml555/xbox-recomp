# MCLA Recomp — Android

Recompilación estática de **Midnight Club: Los Angeles Complete Edition** (Xbox 360) para dispositivos Android ARM64.

Este proyecto recompila el código nativo de Xbox 360 (PowerPC) a ARM64 nativo para Android utilizando la tecnología de recompilación estática ReXGlue. El objetivo es ejecutar MCLA en dispositivos Android con GPU Adreno 7xx/8xx (incluyendo Adreno 710 en Snapdragon 888/888+, 8 Gen 1/2/3 y 8 Elite) a 30+ FPS estables.

## Estado actual

*Actualizado: 20 de noviembre de 2026 (auditoría técnica)*

✅ **SDK ReXGlue v0.10.0** - Buildado para Android ARM64 con Vulkan (M1 completado)
✅ **Codegen M2 completo** - 122 `.cpp` + 123 `.h` (251 archivos en total) con 29,797 funciones recompiladas; 0 stubs/TODO en código generado
✅ **Inicialización verificada** - `xstart` symbol en dirección `0x821322B8` (cadena XEX → config → init → register alineada)
✅ **Integración M3 completa y compilando** - 120 particiones `larecomp_recomp.N.cpp` enlazadas en `libmcla_main.so` (34.6 MB); JNI exporta 16 símbolos `Java_com_mcla_recomp_MainActivity_*`
✅ **APK debug generado** - `m3-android/app/build/outputs/apk/debug/app-debug.apk` (55 MB)
🔄 **Pendiente (M3 ~80%)** - hooks de juego aún stub (283 funciones), updater/shader generator/Vulkan catalog con TODOs
⏳ **Próximo hito: M4** - Boot test en hardware real (Motorola Edge 50 Fusion)

## Requisitos

### Hardware
- **Procesador:** ARMv8.2 con soporte FP16 (Snapdragon 888/888+, 8 Gen 1/2/3 o 8 Elite)
- **RAM:** 8 GB mínimo
- **GPU:** Adreno 7xx/8xx con soporte Vulkan (Adreno 710 compatible con Vulkan 1.1 + Turnip)
- **Almacenamiento:** 10 GB libres

### Software
- **Android:** 7.0 (API 24) o superior
- **NDK:** 26.1.10909125 o superior
- **CMake:** 3.25 o superior
- **JDK:** 17 o superior
- **Gradle:** 8 o superior

## Cómo construir

### Pasos de build

```bash
# 1. Clonar repositorio (con submodules)
git clone --recurse-submodules <repo-url>
cd xbox-360-recomps

# 2. Si no usaste --recurse-submodules, inicializar submodules
git submodule update --init --recursive

# 3. Configurar entorno Android NDK
export ANDROID_NDK_HOME=/ruta/a/android-ndk-r26b
export PATH=$PATH:$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin

# 4. Construir SDK ReXGlue para Android
cd rexglue-build-android
cmake -B build -S .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24
cmake --build build -- -j$(nproc)

# 5. Construir APK
cd ../m3-android
./gradlew assembleDebug
# o para release:
./gradlew assembleRelease
# APK debug resultante: app/build/outputs/apk/debug/app-debug.apk (~55 MB)
```

> **Nota:** Los archivos generados por codegen (m2-codegen/generated/) y el XEX del juego deben estar presentes antes de construir.

## Estructura del proyecto

```
xbox-360-recomps/
├── README.md              # Esta documentación
├── LICENSE                # GPL-3.0 License
├── CHANGELOG.md           # Historial de cambios
├── PLAN.md                # Hoja de ruta del proyecto
├── TIMELINE.md            # Cronología del desarrollo
├── ISSUES.md              # Problemas conocidos
├── GLOSSARY.md            # Glosario de términos
├── patches/               # Parches para ReXGlue
│   ├── rexglue-v0.10.0-mcla.patch
│   ├── rexglue-v0.10.0-android-ui.patch
│   └── ...
├── rexglue-sdk/           # SDK ReXGlue (submodule)
├── rexglue-build-android/ # Build SDK Android (gitignored)
├── rexglue-build-host/    # Build SDK Host (gitignored)
├── m2-codegen/            # Generación de código
│   ├── generated/         # Archivos C++ generados (gitignored)
│   ├── lareconfig.toml    # Configuración del codegen
│   └── debug_magic.py     # Scripts de debugging
├── m3-android/            # Aplicación Android
│   ├── app/
│   │   ├── jni/src/      # Código nativo (C++)
│   │   └── src/main/     # Código Java/Kotlin
│   └── app/build/outputs/apk/debug/  # APK debug (app-debug.apk, gitignored)
├── tools/                 # Herramientas de desarrollo (gitignored)
│   ├── build_host_rexglue.sh
│   └── ...
└── larecomp-ref/          # Referencia LARecomp (submodule)
```

### Submódulos

Este repositorio incluye dos submódulos:
- `rexglue-sdk/`: SDK de recompilación estática ReXGlue
- `larecomp-ref/`: Referencia de LARecomp

Para inicializarlos después de clonar:
```bash
git submodule update --init --recursive
```

## Cómo jugar

1. **Instalar APK** en dispositivo compatible (Adreno 7xx/8xx: Snapdragon 888/888+, 8 Gen 1/2/3, 8 Elite)
2. **Abrir app** y conceder permisos de almacenamiento
3. **Seleccionar juego** desde el menú principal
4. **Configurar controles** (táctil o gamepad físico)
5. **Jugar** — Free roam disponible

### Controles

- **Gamepad:** Soporte nativo vía SDL3 (XInput)
- **Táctil:** Configurables en opciones
- **Volante:** Soporte experimental

### Salida de video

- **API:** OpenGL ES 3.0 (GLSurfaceView)
- **Resolución:** 50-75% por defecto (evitar thermal throttling)
- **FPS objetivo:** 30+ FPS estables

### Salida de audio

- **Cadena:** AAudio (nativo, API 26+) → SDL3 (Xenia) → Null (silencioso, mantiene timing)
- **Formato:** 48 kHz, 256 samples/frame (5.33 ms), 6-ch BE float → stereo float downmix
- **Self-test:** `AAUDIO_SELFTEST` al arranque verifica compatibilidad AAudio (ver [docs/AUDIO.md](docs/AUDIO.md))
- **Quirks:** Motorola Edge 50 Fusion rechaza `LOW_LATENCY`; se usa modo `NONE` por defecto

## Licencia

Este proyecto está licenciado bajo **GPL-3.0**, igual que LARecomp.

Ver [LICENSE](LICENSE) para el texto completo de la licencia.

## Créditos

### Proyecto original
- **LARecomp** — [mzzvxm](https://github.com/mzzvxm/LARecomp)
- **mcla-recomp** — [holdmysocks](https://github.com/holdmysocks/mcla-recomp)

### Equipo MCLA Recomp Android
- **Desarrollo principal:** Coder
- **Verificación:** Control de Calidad
- **Documentación:** Documentador
- **Traducción:** Traductor

### Herramientas
- **ReXGlue v0.10.0** — SDK de recompilación estática
- **SDL3** — Input y windowing
- **OpenGL ES 3.0** — Rendering

## Nota importante

Este proyecto requiere una copia legal del juego original Xbox 360. El archivo XEX debe ser extraído de un disco comprado por el usuario. No distribuimos ni apoyamos ningún método de obtención ilegal de juegos.