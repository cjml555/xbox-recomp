# MCLA Recomp — Android

Recompilación estática de **Midnight Club: Los Angeles Complete Edition** (Xbox 360) para dispositivos Android ARM64.

Este proyecto recompila el código nativo de Xbox 360 (PowerPC) a ARM64 nativo para Android utilizando la tecnología de recompilación estática ReXGlue. El objetivo es ejecutar MCLA en dispositivos Android con GPU Adreno (Snapdragon 8 Gen 1/2/3 o 8 Elite) a 30+ FPS estables.

## Estado actual

✅ **SDK ReXGlue v0.10.0** - Buildado para Android ARM64 con Vulkan (M1 completado)
✅ **APK generado** - `/m3-android/build/app.apk` (268 MB)
✅ **Funciones generadas** - 251 archivos C++ desde M2 codegen
✅ **Inicialización verificada** - `xstart` symbol en dirección `0x821322B8`

## Requisitos

### Hardware
- **Procesador:** ARMv8.2 con soporte FP16 (Snapdragon 8 Gen 1/2/3 o 8 Elite)
- **RAM:** 8 GB mínimo
- **GPU:** Adreno 7xx/8xx con soporte Vulkan
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
cd ../../m3-android
./gradlew assembleDebug
# o para release:
./gradlew assembleRelease
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
│   └── build/            # Build artifacts (gitignored)
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

1. **Instalar APK** en dispositivo compatible (Snapdragon 8 Gen 1/2/3)
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