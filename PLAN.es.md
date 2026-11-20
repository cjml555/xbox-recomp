# Plan de Proyecto: MCLA Recomp — Android

**Coordinador:** plan inicial basado en investigación (18 nov 2026)
**Juego:** Midnight Club: Los Angeles Complete Edition (Xbox 360, title `545407F8`)
**Enfoque:** Recompilación estática (ReXGlue) → nativo ARM64
**Dispositivo objetivo:** Android 16, GPU Adreno (Snapdragon 8 Gen 1/2/3 o 8 Elite), RAM mínima 8 GB, ARMv8.2 (FP16)

## Decisiones técnicas (fijadas)

| Aspecto | Decisión | Justificación |
|---|---|---|
| SDK | ReXGlue v0.10.0 | Ya probado en MCLA (LARecomp) y en Android (Dante's Inferno Android) |
| Base de referencia | LARecomp (mzzvxm) + mcla-recomp (holdmysocks) | Hooks, configs, fixes de motor para MCLA |
| Arquitectura Android | Dantes-inferno-Android (vladector) | APK con NDK + Vulkan + Turnip + AAudio ya probado en Adreno |
| GPU | Backend Vulkan + drivers Turnip (Mesa) vía AdrenoTools | Requisito para Adreno 7xx/8xx en Android 16 |
| Toolchain | NDK 26.1.10909125, Clang, CMake ≥3.25, Ninja | Compatible con ReXGlue y Android 16 |
| Build | Gradle (APK) + CMake/NDK | Entrega instalable |
| Input | SDL3 + gamepad (gamecontrollerdb) + controles táctiles | Necesario: móvil |
| Audio | AAudio nativo (buffer 4096) | Bajo latencia, probado en Dante's Inferno Android |
| ISO | Storage Access Framework, zero-copy via fd (`/proc/self/fd/<fd>`) | Evita duplicar ~8 GB |
| Workaround SDK | `clock_no_scaling=true` | Evita colapso de FPS por vblank (bug conocido del SDK) |

## Hitos

- **M0 — Entorno:** Instalar/verificar NDK, CMake, Ninja, JDK, Android SDK, Turnip. ✅ Criterio: toolchain lista y build de prueba compila.
- **M1 — SDK ReXGlue ARM64:** Portar build de ReXGlue v0.10.0 a NDK (arm64-v8a) con Vulkan. ✅ Criterio: SDK buildable y `rexglue codegen` funcional.
- **M2 — Codegen MCLA:** Extraer XEX del ISO (usuario), codegen del MCLA con config de LARecomp adaptada. ✅ Criterio: código C++ generado completo, sin funciones fallidas críticas.
- **M3 — App Android:** Shell APK (SDL3, AAudio, input, storage/ISO, pantalla de carga). ✅ Criterio: APK instala y arranca.
- **M4 — Boot en dispositivo:** Juego arranca hasta título. ✅ Criterio: título visible en device real Adreno/Android 16.
- **M5 — Jugabilidad y tuning:** Free roam jugable, FPS ≥30, controles, audio, saves. ✅ Criterio: 30 min de sesión sin crash.
- **M6 — QA + entrega:** APK release, docs, traducción, gráficos. ✅ Criterio: todo verificado por QA.

## Riesgos y mitigación

1. **Térmico (crítico):** Snapdragon throttling → limitar resolución por defecto (50-75%), monitorizar, opciones de rendimiento.
2. **eDRAM/tile rendering:** Formatos custom de Xenos → probar fallbacks; referencias: fixes de LARecomp (Patch_SingleTile).
3. **Stutter de shaders:** Cache persistente en disco (`MESA_DISK_CACHE_SINGLE_FILE=1`) + precache.
4. **Input:** Mapeo de controles táctiles configurable + gamepad.
5. **MMIO / funciones perdidas:** Hook de runtime + `runtime_discovered.toml` como en mcla-recomp.
6. **Dithered alpha en sombras:** Conocido en LARecomp; aceptar como issue abierta.

## Equipo y asignación

- **Coder:** M0→M5 (todo el código, tests, build, debug)
- **Control de Calidad:** Verifica cada hito (builds, comportamiento en device, gráficos, docs). Sin verificación no se acepta.
- **Marketing/Gráficos:** Icono, banner, capturas, presentación. Diseño aprobado por el usuario.
- **Documentador:** README, changelog, línea de tiempo, seguimiento de issues.
- **Traductor:** Traducción de docs y comentarios (ES/EN).

## Reglas activas

- Código sin probar no cuenta (build + test obligatorio en cada hito).
- QA verifica todo.
- El usuario aprueba el diseño gráfico.
- Si algo es necesario para el trabajo, se instala.
- Respuestas cortas y concisas.
