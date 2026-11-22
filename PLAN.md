# Plan de Proyecto: MCLA Recomp — Android

**Coordinador:** plan inicial basado en investigación (18 nov 2026)
**Juego:** Midnight Club: Los Angeles Complete Edition (Xbox 360, title `545407F8`)
**Enfoque:** Recompilación estática (ReXGlue) → nativo ARM64
**Dispositivo objetivo:** Android 16, GPU Adreno 7xx/8xx (Snapdragon 888/888+ [Adreno 710], 8 Gen 1/2/3, 8 Elite), RAM mínima 8 GB, ARMv8.2 (FP16)

## Decisiones técnicas (fijadas)

| Aspecto | Decisión | Justificación |
|---|---|---|
| SDK | ReXGlue v0.10.0 | Ya probado en MCLA (LARecomp) y en Android (Dante's Inferno Android) |
| Base de referencia | LARecomp (mzzvxm) + mcla-recomp (holdmysocks) | Hooks, configs, fixes de motor para MCLA |
| Arquitectura Android | Dantes-inferno-Android (vladector) | APK con NDK + Vulkan + Turnip + AAudio ya probado en Adreno |
| GPU | Backend Vulkan + drivers Turnip (Mesa) vía AdrenoTools | Requisito para Adreno 7xx/8xx en Android 16. Adreno 710 (SD 888) compatible Vulkan 1.1 + Turnip |
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
- **M4 — Boot en dispositivo:** Juego arranca hasta título. ✅ Criterio: título visible en device real Adreno/Android 16. Dispositivos probados: Adreno 710 (SD 888/888+).
- **M5 — Jugabilidad y tuning:** Free roam jugable, FPS ≥30, controles, audio, saves. ✅ Criterio: 30 min de sesión sin crash.
- **M6 — QA + entrega:** APK release, docs, traducción, gráficos. ✅ Criterio: todo verificado por QA.

## Roadmap de Features

Spec completo en `FEATURES.md` (alcance final del usuario, 20 nov 2026). Orden de prioridad explícito: **renderizadores → jugable → optimización → texturas**. Los hitos M0–M6 se mantienen; las fases se apoyan en ellos.

- **F1 — Renderizadores → jugable (prioridad máxima):** implementar los renderizadores ANTES de cualquier mejora; el juego debe cargar, renderizar y ser jugable (entrar al juego). Apoyado en M2–M5.
  - Selector de archivos del juego (cargar y entrar al juego).
  - Selector de renderizador: basado en Xenia + nativo/Vulkan.
  - Selector de API gráfica.
  - Generador de shaders integrado (multi-dispositivo).
  - Soporte de gamepads físicos (Xbox, PlayStation, similares).
  - Gamepad virtual táctil (visible/funcional sin gamepad físico).
  - Modo de desarrollo (depuración del render y boot).
- **F2 — Optimización (solo después de que renderice):** optimización del código y de la carga de archivos; resto de menús y features del app. Apoyado en M5–M6.
  - Optimización de código y de carga de archivos.
  - Medidor FPS tipo MangoHud.
  - Loader de drivers Vulkan públicos + catálogo de drivers públicos y custom.
  - Configuración de calidad con perfiles: bajo, medio, alto, ultra, custom (configs del juego).
  - Menú de partida personalizada (custom save).
  - Updater / verificador de actualizaciones.
  - Selector de idiomas.
  - Menú de cheats.
  - Menú de mods.
  - Modo LAN con gamertag personalizada (de ser posible — condicional).
  - Créditos.
  - Botón para enviar logs de crash al dev.
- **F3 — Investigación de texturas (solo al terminar F2):** investigar cómo mejorar las texturas del juego y aplicar las mejoras viables.

**Restricción legal (todas las fases):** los datos del juego NO se suben ni publican; SÍ se permiten scripts para parchar o descomprimir los archivos del juego.

## Riesgos y mitigación

1. **Térmico (crítico):** Snapdragon throttling → limitar resolución por defecto (50-75%), monitorizar, opciones de rendimiento. **Adreno 710 (SD 888/888+):** throttling más severo que Gen 1-3; resolución inicial recommendation 40-50%, thermal throttle threshold 45°C.
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
