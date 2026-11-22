# Changelog

Todos los cambios notables a este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/).

## [No lanzado]

### Auditoría técnica completa (2026-11-20)
- ✅ **M2 verificado (~95%):** codegen completo sobre `/home/jhona/MCLA/default.xex` (9.2 MB). 251 archivos generados (122 `.cpp` + 123 `.h` + 2 `.cmake` + 4 stamps/config), 29,797 funciones (`DEFINE_REX_FUNC`/`REX_FUNC_PROLOGUE` 1:1), 0 stubs/TODO en código generado
- ✅ Entry point `xstart = 0x821322B8` alineado en toda la cadena (XEX header → config → init → register → recomp.60)
- ✅ **ISSUE #008 (XEX) RESUELTO:** archivo proporcionado por el usuario; también copiado a `m2-codegen/assets/default.xex`
- ✅ **ISSUE #007 (libtinfo.so.5) verificado:** ya no bloquea — el binario host `rexglue-sdk/out/linux-amd64/rexglue` v0.10.0 ejecuta sin la librería (`ldd` limpio) y el codegen se completó con él. El sistema solo tiene `libtinfo.so.6`; un rebuild host con Clang 18 podría requerir de nuevo el workaround
- ✅ **M3 verificado (~80%):** integración completa y compilando. `app-debug.apk` (55 MB) en `m3-android/app/build/outputs/apk/debug/`; `libmcla_main.so` 34.6 MB (stripped) con las 120 particiones `larecomp_recomp.N.cpp` vía CMake GLOB; 16 símbolos JNI `Java_com_mcla_recomp_MainActivity_*` exportados
- ✅ Blockers previos de M2 (nombre de PCH y estándar C++) confirmados como resueltos
- ⚠️ **Gaps funcionales detectados en M3:** `game_hooks.cpp` con 283 funciones stub (crítico para M4), `updater.cpp` sin parseo JSON, TODOs en `shader_generator.cpp` (binario de shader) y `vulkan_catalog.cpp` (instalación/desinstalación de drivers)
- ⚠️ **Nuevas issues abiertas:** #009 hooks stub (crítica M4), #010 rutas absolutas `/home/jhona/...` en CMake (portabilidad), #011 manifest exige GLES 3.0/AEP `required=true` en app Vulkan-only, #012 inconsistencia C++20 (`CMAKE_CXX_STANDARD`) vs C++23 (`cxx_std_23`)
- 📝 Documentación actualizada al estado real: PROGRESS.md, README.md, CHANGELOG.md, TIMELINE.md, ISSUES.md (ruta y tamaño de APK corregidos; conteo de archivos codegen precisado)
- ⏳ **Próximo hito M4:** boot test en Motorola Edge 50 Fusion (conectado)

### M3 — Integración Android (2026-11-19/20)
- ✅ Código generado integrado en `libmcla_main.so` mediante CMake GLOB de `larecomp_recomp.*.cpp`
- ✅ JNI bridge funcional con 16 símbolos exportados hacia `MainActivity`
- ✅ APK debug compilado y generado (55 MB)
- 🔄 Pendiente: implementación real de hooks, updater, shaders y catálogo Vulkan

### M2 — Codegen completado (2026-11-19)
- ✅ XEX recibido y codegen ejecutado: 29,797 funciones recompiladas en 120 particiones
- ✅ Verificación estructural completa (M2_VERIFICATION_REPORT.md): entry point alineado, sin stubs/TODO
- 🔄 Pendiente: validación de funciones fallidas críticas y aplicación de hacks LARecomp/mcla-recomp

### M2 — Codegen en verificación (2026-11-19)
- ✅ Submódulos clonados e inicializados: `rexglue-sdk` (v0.10.0) y `larecomp-ref`
- 🔄 Inicio de verificación de M2 (codegendel MCLA con configuración LARecomp adaptada)
- ⚠️ Bloqueo crítico: archivo XEX del juego no disponible para extracción
- ⚠️ Bloqueo medio: dependencia libtinfo.so.5 faltante para Clang 18.1.8 en build host

### M0 — Entorno (2026-11-18)
- ✅ Toolchain instalada y verificada (NDK, CMake, Ninja, JDK, Android SDK, Turnip)
- ✅ Criterio de aceptación cumplido: build de prueba compila correctamente

### M1 — ReXGlue SDK ARM64 (2026-11-18)
- ✅ ReXGlue v0.10.0 buildado para `android-arm64` con Vulkan habilitado
- ✅ `librexruntime.so` generado (86.5 MB, compatible Android 24+)
- ✅ SDL3 video stubs creados para Android (SDL_VIDEO_DISABLED)
- ✅ Corregido error de link (`-lpthread`, `-lrt`) en Android bionic
- ✅ Toolchain verificado: NDK 26.1.10909125, Clang 17.0.2, CMake 3.31.0, Ninja 1.12.1

### [0.0.0] — 2026-11-18
#### Añadido
- Documento PLAN.md con decisiones técnicas iniciales
- Definición de hitos (M0→M6)
- Identificación de riesgos y mitigaciones
- Asignación de roles del equipo

#### Decisiones técnicas fijadas
- SDK: ReXGlue v0.10.0
- Base de referencia: LARecomp (mzzvxm) + mcla-recomp (holdmysocks)
- Arquitectura Android: Dantes-inferno-Android (vladector)
- GPU: Backend Vulkan + drivers Turnip (Mesa) vía AdrenoTools
- Toolchain: NDK 26.1.10909125, Clang, CMake ≥3.25, Ninja
- Input: SDL3 + gamepad + controles táctiles
- Audio: AAudio nativo (buffer 4096)
- ISO: Storage Access Framework, zero-copy via fd

---

## Formato

### Añadido
Para nuevas funcionalidades.

### Cambiado
Para cambios en funcionalidades existentes.

### Eliminado
Para funcionalidades removidas.

### Corregido
Para correcciones de bugs.

### Seguridad
En caso de vulnerabilidades.
