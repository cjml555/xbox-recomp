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

## Próximos Hitos

### M1 — SDK ReXGlue ARM64
- **Criterio de aceptación:** SDK buildable y `rexglue codegen` funcional
- **Estado:** ✅ COMPLETADO (18 nov 2026)
- **Detalles:** ReXGlue v0.10.0 buildado para `android-arm64` con Vulkan. `librexruntime.so` generado (86.5 MB, compatible Android 24+). SDL3 stubs creados para video disable en Android. Toolchan NDK 26.1.10909125, Clang 17.0.2, CMake 3.31.0, Ninja 1.12.1.

### M2 — Codegen MCLA
- **Criterio de aceptación:** Código C++ generado completo, sin funciones fallidas críticas
- **Estado:** Pendiente

### M3 — App Android
- **Criterio de aceptación:** APK instala y arranca
- **Estado:** Pendiente

### M4 — Boot en dispositivo
- **Criterio de aceptación:** Título visible en device real Adreno/Android 16
- **Estado:** Pendiente

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
