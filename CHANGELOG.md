# Changelog

Todos los cambios notables a este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/).

## [No lanzado]

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
