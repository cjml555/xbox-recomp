# Rastreador de Issues

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
- **Referencia:** PLAN.es.md §Riesgos #1

### #002 — eDRAM / tile rendering custom de Xenos
- **Estado:** Abierto
- **Prioridad:** Alta
- **Responsable:** Coder
- **Descripción:** La GPU Xenos de Xbox 360 usa formatos de eDRAM y tile rendering custom que requieren emulación.
- **Mitigación:**
  - Implementar fallbacks para formatos no soportados
  - Reutilizar fixes de LARecomp (Patch_SingleTile)
- **Referencia:** PLAN.es.md §Riesgos #2

### #003 — Stutter de shaders en primera carga
- **Estado:** Abierto
- **Prioridad:** Alta
- **Responsable:** Coder
- **Descripción:** Compilación de shaders en runtime causa stutter inicial.
- **Mitigación:**
  - Caché persistente en disco (`MESA_DISK_CACHE_SINGLE_FILE=1`)
  - Precache de shaders comunes
- **Referencia:** PLAN.es.md §Riesgos #3

### #004 — Mapeo de controles táctiles
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** Necesario implementar controles táctiles configurables para dispositivos sin gamepad.
- **Mitigación:**
  - UI de configuración de controles
  - Perfiles predefinidos
  - Soporte simultáneo táctil + gamepad
- **Referencia:** PLAN.es.md §Riesgos #4

### #005 — Funciones MMIO no mapeadas
- **Estado:** Abierto
- **Prioridad:** Media
- **Responsable:** Coder
- **Descripción:** Algunas funciones de hardware pueden no estar mapeadas inicialmente.
- **Mitigación:**
  - Hook de runtime para funciones descubiertas
  - Archivo `runtime_discovered.toml` (como en mcla-recomp)
- **Referencia:** PLAN.es.md §Riesgos #5

### #006 — Dithered alpha en sombras (artístico)
- **Estado:** Abierto
- **Prioridad:** Baja
- **Responsable:** Coder
- **Descripción:** Artefacto visual conocido en sombras con alpha dithered.
- **Mitigación:** Aceptar como limitación conocida (presente en LARecomp)
- **Referencia:** PLAN.es.md §Riesgos #6

---

## Historial de Issues Cerradas

| ID | Título | Estado | Fecha de cierre |
|----|--------|--------|-----------------|
| — | — | — | — |

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
