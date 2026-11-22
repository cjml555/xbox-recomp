# Línea de Tiempo del Proyecto

Registro cronológico de hitos y decisiones del proyecto MCLA Recomp — Android.

---

## 2026

### Noviembre 2026

#### 18 de Noviembre — Inicio del Proyecto (M0)

**Hito:** M0 — Entorno completado ✅

**Decisiones tomadas:**
- **SDK seleccionado:** ReXGlue v0.10.0
  - Justificación: Ya probado en MCLA (LARecomp) y en Android (Dante's Inferno Android)
  
- **Base de referencia:** LARecomp (mzzvxm) + mcla-recomp (holdmysocks)
  - Justificación: Hooks, configs, fixes de motor específicos para MCLA
  
- **Arquitectura Android:** Dantes-inferno-Android (vladector)
  - Justificación: APK con NDK + Vulkan + Turnip + AAudio ya probado en Adreno
  
- **GPU Backend:** Vulkan + drivers Turnip (Mesa) vía AdrenoTools
  - Justificación: Requisito para GPUs Adreno 7xx/8xx en Android 16
  
- **Toolchain:** NDK 26.1.10909125, Clang, CMake ≥3.25, Ninja
  - Justificación: Compatible con ReXGlue y Android 16
  
- **Sistema de Input:** SDL3 + gamepad (gamecontrollerdb) + controles táctiles
  - Justificación: Necesario para soporte móvil completo
  
- **Audio:** AAudio nativo con buffer de 4096 samples
  - Justificación: Baja latencia, probado en Dante's Inferno Android
  
- **Acceso a ISO:** Storage Access Framework, zero-copy via `/proc/self/fd/<fd>`
  - Justificación: Evita duplicar ~8 GB de datos
  
- **Workaround SDK:** `clock_no_scaling=true`
  - Justificación: Evita colapso de FPS por vblank (bug conocido del SDK)

**Dispositivo objetivo definido:**
- Android 16
- GPU Adreno (Snapdragon 8 Gen 1/2/3 o 8 Elite)
- RAM mínima 8 GB
- ARMv8.2 con soporte FP16

**Riesgos identificados:**
1. Thermal throttling en Snapdragon (Crítico)
2. eDRAM / tile rendering custom de Xenos (Alto)
3. Stutter de shaders en primera carga (Alto)
4. Mapeo de controles táctiles (Medio)
5. Funciones MMIO no mapeadas (Medio)
6. Dithered alpha en sombras (Bajo)

**Equipo asignado:**
- Coder: M0→M5 (todo el código, tests, build, debug)
- Control de Calidad: Verifica cada hito
- Marketing/Gráficos: Icono, banner, capturas, presentación
- Documentador: README, changelog, línea de tiempo, issues
- Traductor: Traducción de docs y comentarios (ES/EN)

**Reglas activas establecidas:**
- Código sin probar no cuenta (build + test obligatorio)
- QA verifica todo antes de aceptar
- El usuario aprueba el diseño gráfico
- Si algo es necesario para el trabajo, se instala
- Respuestas cortas y concisas

**Documentación creada:**
- PLAN.md — Plan inicial del proyecto
- README.md — Documentación base
- CHANGELOG.md — Registro de cambios
- ISSUES.md — Tracker de issues
- TIMELINE.md — Línea de tiempo (este documento)

---

#### 19 de Noviembre — Codegen MCLA completado (M2) e integración Android (M3)

**Hito:** M2 — Codegen ~95% 🔄 · M3 — Integración ~80% 🔄

**Eventos:**
- XEX del juego recibido del usuario: `/home/jhona/MCLA/default.xex` (9,252,864 bytes ≈ 9.2 MB, título `545407F8`) → **ISSUE #008 desbloqueada**
- `rexglue codegen` ejecutado: 251 archivos generados (122 `.cpp` + 123 `.h` + cmake/config), **29,797 funciones** recompiladas en 120 particiones
- Verificación estructural M2 (M2_VERIFICATION_REPORT.md): entry point `xstart = 0x821322B8` alineado en toda la cadena; 0 stubs/TODO en código generado
- Integración del código generado en `m3-android`: CMake GLOB de `larecomp_recomp.*.cpp` → `libmcla_main.so`; JNI bridge con 16 símbolos `Java_com_mcla_recomp_MainActivity_*`
- APK debug compilado: `m3-android/app/build/outputs/apk/debug/app-debug.apk` (55 MB)

**Notas:** Quedan pendientes en M2 la validación de funciones fallidas críticas y los hacks de LARecomp/mcla-recomp.

---

#### 20 de Noviembre — Auditoría técnica completa

**Hito:** Auditoría de estado real M0–M3 ✅

**Hallazgos verificados:**
- M0–M2 confirmados: codegen completo y compilable; los 2 blockers de M2 (nombre de PCH y estándar C++) ya resueltos
- **ISSUE #008 (XEX): RESUELTA** — archivo disponible y codegen ejecutado sobre él
- **ISSUE #007 (libtinfo.so.5): ya no bloquea** — el binario host `rexglue` v0.10.0 ejecuta sin la librería (`ldd` limpio); el sistema solo tiene `libtinfo.so.6`
- M3 compilando con APK debug de 55 MB y `libmcla_main.so` de 34.6 MB (stripped)
- Documentación encontrada DESACTUALIZADA (ruta/tamaño de APK erróneos, conteo de archivos impreciso, M2/M3 marcados como pendientes) → corregida en PROGRESS.md, README.md, CHANGELOG.md, TIMELINE.md e ISSUES.md

**Gaps funcionales detectados (nuevas issues #009–#012):**
1. `game_hooks.cpp`: 283 funciones hook stub (crítico para M4) — #009
2. Rutas absolutas `/home/jhona/...` en CMake (portabilidad) — #010
3. Manifest exige GLES 3.0/AEP `required=true` en app Vulkan-only — #011
4. Inconsistencia C++20 (`CMAKE_CXX_STANDARD`) vs C++23 (`cxx_std_23`) — #012
5. `updater.cpp` sin parseo JSON; TODOs en `shader_generator.cpp` y `vulkan_catalog.cpp`

**Dispositivo de pruebas:** Motorola Edge 50 Fusion conectado y listo para M4.

**Notas:** Próximo hito M4 — boot test en hardware real.

---

## Próximos Hitos

### M1 — SDK ReXGlue ARM64
- **Criterio de aceptación:** SDK buildable y `rexglue codegen` funcional
- **Estado:** ✅ COMPLETADO (18 nov 2026)
- **Detalles:** ReXGlue v0.10.0 buildado para `android-arm64` con Vulkan. `librexruntime.so` generado (86.5 MB, compatible Android 24+). SDL3 stubs creados para video disable en Android. Toolchan NDK 26.1.10909125, Clang 17.0.2, CMake 3.31.0, Ninja 1.12.1.

### M2 — Codegen MCLA
- **Criterio de aceptación:** Código C++ generado completo, sin funciones fallidas críticas
- **Estado:** 🔄 ~95% (codegen completo y verificado 19–20 nov 2026; falta validación de funciones fallidas y hacks LARecomp)
- **Detalles:** 251 archivos (122 `.cpp` + 123 `.h` + cmake/config), 29,797 funciones, entry point `xstart = 0x821322B8` alineado, 0 stubs/TODO en código generado.

### M3 — App Android
- **Criterio de aceptación:** APK instala y arranca
- **Estado:** 🔄 ~80% (integración completa y compilando, 20 nov 2026)
- **Detalles:** `app-debug.apk` (55 MB) en `m3-android/app/build/outputs/apk/debug/`; `libmcla_main.so` 34.6 MB con 120 particiones recompiladas; 16 símbolos JNI exportados. Gaps: hooks stub, updater, shaders, Vulkan catalog (issues #009–#012).

### M4 — Boot en dispositivo
- **Criterio de aceptación:** Título visible en device real Adreno/Android 16
- **Estado:** ⏳ Próximo hito — Motorola Edge 50 Fusion conectado para boot test

### M5 — Jugabilidad y tuning
- **Criterio de aceptación:** 30 min de sesión sin crash, FPS ≥30
- **Estado:** Pendiente

### M6 — QA + entrega
- **Criterio de aceptación:** APK release, docs, traducción, gráficos verificados
- **Estado:** Pendiente

---

## Formato de Entrada

```markdown
### DD de Mes AAAA — Descripción del evento

**Hito:** [Nombre del hito] [Estado]

**Decisiones tomadas:**
- [Decisión]
  - Justificación: [Razón]

**Notas:** [Cualquier nota relevante]
```
