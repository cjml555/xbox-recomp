# Tracker de Issues

Seguimiento de issues abiertas del proyecto MCLA Recomp — Android.

## Issues Activas

| ID | Título | Estado | Prioridad | Responsable |
|----|--------|--------|-----------|-------------|
| #001 | Thermal throttling en Snapdragon | Abierto | Crítica | Coder |
| #002 | eDRAM / tile rendering custom de Xenos | Abierto | Alta | Coder |
| #003 | Stutter de shaders en primera carga | Abierto | Alta | Coder |
| #004 | Mapeo de controles táctiles | Abierto | Media | Coder |
| #005 | Funciones MMIO no mapeadas | Abierto | Media | Coder |
| #006 | Dithered alpha en sombras (artístico) | Abierto | Baja | Coder |
| #009 | game_hooks.cpp: 283 funciones hook stub (bloquea M4) | Abierto | Crítica | Coder |
| #010 | Rutas absolutas en CMake (portabilidad) | Abierto | Alta | Coder |
| #011 | Manifest exige GLES 3.0/AEP required en app Vulkan-only | Abierto | Media | Coder |
| #012 | Inconsistencia C++20 vs C++23 en CMake de la app | Abierto | Media | Coder |

---

## Detalle de Issues

### #001 — Thermal throttling en Snapdragon
- **Estado:** Abierto
- **Prioridad:** Crítica
- **Responsable:** Coder
- **Descripción:** Los dispositivos Snapdragon sufren throttling térmico que reduce FPS drásticamente.
- **Mitigación:**
  - Limitar resolución por defecto (50-75%)
  - Monitorizar temperatura en runtime
  - Opciones de rendimiento configurables
- **Referencia:** PLAN.md §Riesgos #1

### #002 — eDRAM / tile rendering custom de Xenos
- **Estado:** Abierto
- **Prioridad:** Alta
- **Responsable:** Coder
- **Descripción:** La GPU Xenos de Xbox 360 usa formatos de eDRAM y tile rendering custom que requieren emulación.
- **Mitigación:**
  - Implementar fallbacks para formatos no soportados
  - Reutilizar fixes de LARecomp (Patch_SingleTile)
- **Referencia:** PLAN.md §Riesgos #2

### #003 — Stutter de shaders en primera carga
- **Estado:** Abierto
- **Prioridad:** Alta
- **Responsable:** Coder
- **Descripción:** Compilación de shaders en runtime causa stutter inicial.
- **Mitigación:**
  - Caché persistente en disco (`MESA_DISK_CACHE_SINGLE_FILE=1`)
  - Precache de shaders comunes
- **Referencia:** PLAN.md §Riesgos #3

### #004 — Mapeo de controles táctiles
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** Necesario implementar controles táctiles configurables para dispositivos sin gamepad.
- **Mitigación:**
  - UI de configuración de controles
  - Perfiles predefinidos
  - Soporte simultáneo táctil + gamepad
- **Referencia:** PLAN.md §Riesgos #4

### #005 — Funciones MMIO no mapeadas
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** Algunas funciones de hardware pueden no estar mapeadas inicialmente.
- **Mitigación:**
  - Hook de runtime para funciones descubiertas
  - Archivo `runtime_discovered.toml` (como en mcla-recomp)
- **Referencia:** PLAN.md §Riesgos #5

### #006 — Dithered alpha en sombras (artístico)
- **Estado:** Abierto
- **Prioridad:** Baja
- **Responsable:** Coder
- **Descripción:** Artefacto visual conocido en sombras con alpha dithered.
- **Mitigación:** Aceptar como limitación conocida (presente en LARecomp)
- **Referencia:** PLAN.md §Riesgos #6

### #007 — Dependencia libtinfo.so.5 faltante para Clang 18
- **Estado:** Cerrado (2026-11-20 — verificado: ya no bloquea)
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** El build del SDK ReXGlue para host Linux fallaba durante linking al usar Clang 18.1.8 porque busca `libtinfo.so.5` que no está presente en el sistema (solo `libtinfo.so.6`).
- **Resolución (verificada en auditoría 2026-11-20):** El binario host `rexglue-sdk/out/linux-amd64/rexglue` (v0.10.0) ejecuta correctamente sin `libtinfo.so.5` (`ldd` sin dependencias faltantes) y el codegen de M2 se completó con él. La dependencia ya no bloquea ningún hito activo.
- **Nota:** `libtinfo.so.5` sigue ausente en el sistema; un rebuild host desde cero con Clang 18 podría volver a requerir el workaround (symlink desde `libtinfo.so.6`, paquete compat, o Clang 17).
- **Referencia:** PLAN.md §Riesgos; TIMELINE.md 19–20 nov 2026; BUILD_LOG.md

### #008 — XEX del juego no disponible para codegen
- **Estado:** Cerrado (2026-11-20 — RESUELTO)
- **Prioridad:** Crítica
- **Responsable:** Coordinador
- **Descripción:** No se podía ejecutar `rexglue codegen` sin el archivo binario XEX extraído del disco físico de Midnight Club: Los Angeles Complete Edition (título Xbox `545407F8`).
- **Resolución:** El usuario proporcionó el XEX en `/home/jhona/MCLA/default.xex` (9,252,864 bytes ≈ 9.2 MB; copia de trabajo en `m2-codegen/assets/default.xex`). Codegen ejecutado exitosamente: 251 archivos, 29,797 funciones, entry point `xstart = 0x821322B8` verificado.
- **Referencia:** PLAN.md (requisito M2), PROGRESS.md §Bloqueos Resueltos, M2_VERIFICATION_REPORT.md

### #009 — game_hooks.cpp: funciones hook stub (crítico para M4)
- **Estado:** Abierto
- **Prioridad:** Crítica
- **Responsable:** Coder
- **Descripción:** De las funciones hook de `m3-android/app/jni/src/game_hooks.cpp`, 283 son stubs que solo registran un log ("Hook_X called - stub") sin implementar la lógica del juego (verificado por conteo directo en auditoría 2026-11-20; el informe de auditoría citaba ~452, cifra no reproducida en el fuente actual). Sin hooks reales (save/Load, rendering, input del motor) el boot de M4 no puede llegar a título visible. Además `updater.cpp` tiene el parseo JSON sin implementar y hay TODOs en `shader_generator.cpp` (generación de binario de shader) y `vulkan_catalog.cpp` (instalación/desinstalación de drivers).
- **Mitigación:**
  - Portar los hooks equivalentes de LARecomp/mcla-recomp (`larecomp-ref`) priorizando los críticos para boot
  - Implementar updater (parseo JSON) y shader generator real
  - Completar instalación de drivers Turnip en vulkan_catalog
- **Referencia:** Auditoría 2026-11-20 (CHANGELOG.md, TIMELINE.md); PROGRESS.md §M3 gaps

### #010 — Rutas absolutas en CMake (portabilidad)
- **Estado:** Abierto
- **Prioridad:** Alta
- **Responsable:** Coder
- **Descripción:** `m3-android/app/jni/CMakeLists.txt` contiene rutas absolutas hardcodeadas a `/home/jhona/...` (NDK base, `REXSDK_DIR`, `GENERATED_DIR`, `RUNTIME_LIB_DIR`, `XEX_SOURCE`). El build no es reproducible en otra máquina ni en CI.
- **Mitigación:**
  - Sustituir por variables de entorno / cache variables con defaults relativos (`$ENV{ANDROID_NDK_HOME}`, `${CMAKE_CURRENT_SOURCE_DIR}/../../..`)
  - Documentar las variables requeridas en README.md
- **Referencia:** Auditoría 2026-11-20; m3-android/app/jni/CMakeLists.txt líneas 20, 28, 47, 140, 239

### #011 — Manifest exige GLES 3.0/AEP required en app Vulkan-only
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** `AndroidManifest.xml` declara `glEsVersion="0x00030000" required="true"` y `android.hardware.opengles.aep required="true"`, mientras que `android.hardware.vulkan.version` figura `required="false"`. La app renderiza por Vulkan (+ Turnip); exigir GLES 3.0/AEP como requisito filtra dispositivos compatibles en Play Store y contradice el backend real.
- **Mitigación:**
  - Marcar GLES/AEP como `required="false"` (o eliminarlos si no se usan)
  - Declarar Vulkan como requisito real según el backend soportado
- **Referencia:** Auditoría 2026-11-20; m3-android/app/src/main/AndroidManifest.xml

### #012 — Inconsistencia C++20 vs C++23 en CMake de la app
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** `m3-android/app/jni/CMakeLists.txt` fija `CMAKE_CXX_STANDARD 20` (línea 5) pero exige `target_compile_features(mcla_main PRIVATE cxx_std_23)` (línea 132). El código generado por ReXGlue requiere C++23; la mezcla puede producir flags contradictorios según el orden de evaluación y confundir el estándar real del build.
- **Mitigación:**
  - Unificar en C++23 (`set(CMAKE_CXX_STANDARD 23)`) y eliminar la duplicidad con `cxx_std_23`, o documentar por qué se mantiene ambas
- **Referencia:** Auditoría 2026-11-20; M2_VERIFICATION_REPORT.md (C++23 requerido por el SDK)

---

## Historial de Issues Cerradas

| ID | Título | Estado | Fecha de cierre |
|----|--------|--------|-----------------|
| #007 | Dependencia libtinfo.so.5 faltante para Clang 18 | Cerrado (ya no bloquea; binario host funcional sin la librería) | 2026-11-20 |
| #008 | XEX del juego no disponible para codegen | Cerrado (XEX proporcionado; codegen completado) | 2026-11-20 |

---

## Formato de Nueva Issue

```markdown
### #XXX — Título descriptivo
- **Estado:** Abierto / En progreso / Revisión / Cerrado
- **Prioridad:** Crítica / Alta / Media / Baja
- **Responsable:** [Rol]
- **Descripción:** [Descripción clara del problema]
- **Mitigación:** [Pasos para resolver o workaround]
- **Referencia:** [Enlace a documento relevante]
```
