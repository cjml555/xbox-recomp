# Progreso del Proyecto MCLA Recomp Android

Estado actual al 20 de noviembre de 2026 - Auditoría técnica completa (M0–M3 verificados)

## Resumen Ejecutivo

**Fase actual:** M3 — App Android (integración completa) → M4 Boot en dispositivo  
**Progreso general:** ~55% completado (M0 y M1 finalizados, M2 ~95%, M3 ~80%)  
**Bloqueantes activos:** 0 críticos (XEX y libtinfo resueltos; quedan gaps funcionales de M3)  
**Próximo objetivo:** M4 — Boot test en hardware real (Motorola Edge 50 Fusion conectado)

---

## Estado por Hito

| Hito | Nombre | Estado | Responsable | Fecha inicio | Fecha fin | % | Notas |
|------|--------|--------|-------------|--------------|-----------|---|-------|
| M0 | Entorno | ✅ Completado | Coder | 2026-11-18 | 2026-11-18 | 100% | Toolchain verificada |
| M1 | SDK ReXGlue ARM64 | ✅ Completado | Coder | 2026-11-18 | 2026-11-18 | 100% | librexruntime.so generado |
| M2 | Codegen MCLA | 🔄 En progreso | Coder | 2026-11-19 | — | ~95% | Codegen completo (29,797 funciones); falta validación de funciones fallidas y hacks LARecomp |
| **M3** | **App Android** | **🔄 En progreso** | **Coder** | **2026-11-19** | **—** | **~80%** | **Integración completa y compilando; APK debug 55 MB; gaps funcionales pendientes** |
| M4 | Boot en dispositivo | ⏳ Próximo | Coder | — | — | 0% | Motorola Edge 50 Fusion conectado |
| M5 | Jugabilidad y tuning | ⏳ Pendiente | Coder | — | — | 0% | Meta: 30 min sin crash |
| M6 | QA + entrega | ⏳ Pendiente | QA | — | — | 0% | Entrega final |

---

## Detalle por Hito

### ✅ M0 — Entorno (COMPLETADO)
**Completado:** 18 de noviembre de 2026

- [x] NDK 26.1.10909125 instalado y verificado
- [x] CMake 3.31.0 disponible
- [x] Ninja 1.12.1 disponible
- [x] JDK 17+ configurado
- [x] Android SDK configurado
- [x] Drivers Turnip identificados para Adreno
- [x] Build de prueba compilado exitosamente

**Toolchain verificada:**
- NDK: 26.1.10909125
- Clang: 17.0.2 (ARM64), 18.1.8 (x86_64 host)
- CMake: 3.31.0
- Ninja: 1.12.1

---

### ✅ M1 — SDK ReXGlue ARM64 (COMPLETADO)
**Completado:** 18 de noviembre de 2026

- [x] ReXGlue v0.10.0 construido para `android-arm64`
- [x] Vulkan habilitado en build Android
- [x] `librexruntime.so` generado (86.5 MB)
- [x] Compatible con Android 24+
- [x] SDL3 video stubs creados (`SDL_VIDEO_DISABLED`)
- [x] Correcciones de link aplicadas (`-lpthread`, `-lrt` → bionic)

**Artefactos generados:**
- `rexglue-build-android/librexruntime.so` (86.5 MB)

---

### 🔄 M2 — Codegen MCLA (EN PROGRESO - ~95%)
**Inicio:** 19 de noviembre de 2026  
**Estado:** Codegen completo y verificado por auditoría (2026-11-20); pendiente validación final

**Completado:**
- [x] Submódulo `rexglue-sdk` clonado (v0.10.0)
- [x] Submódulo `larecomp-ref` clonado (referencia completa)
- [x] Configuración (`larecomp_config.toml`) preparada
- [x] Manifest MCLA (`mcla_manifest.toml`) leído
- [x] Scripts de debug validados (`debug_magic.py`, `gdb_break.gdb`)
- [x] XEX disponible: `/home/jhona/MCLA/default.xex` (9,252,864 bytes ≈ 9.2 MB; copia en `m2-codegen/assets/default.xex`) → ISSUE #008 RESUELTO
- [x] `rexglue codegen` ejecutado: **251 archivos generados** (122 `.cpp` + 123 `.h` + 2 `.cmake` + 4 stamps/config)
- [x] **29,797 funciones** recompiladas (`DEFINE_REX_FUNC` / `REX_FUNC_PROLOGUE` 1:1 en 120 particiones)
- [x] Entry point verificado y alineado en toda la cadena: XEX header → config → init → register → `DEFINE_REX_FUNC(xstart)` en `0x821322B8`
- [x] 0 stubs / TODOs / funciones incompletas en código generado
- [x] Blocker de nombre de PCH resuelto
- [x] Blocker de estándar C++ resuelto (código generado compila con C++23 del SDK)

**Pendiente:**
- [ ] Verificar funciones fallidas críticas (revisión funcional, no solo estructural)
- [ ] Aplicar hacks de LARecomp/mcla-recomp al código generado

**Bloqueantes:** ninguno activo (ver §Bloqueos Resueltos)

---

### 🔄 M3 — App Android (EN PROGRESO - ~80%)
**Inicio:** 19 de noviembre de 2026  
**Estado:** Integración COMPLETA y compilando; APK debug generado

**Completado:**
- [x] MainActivity.java + JNI bridge (`jni_bridge.cpp`)
- [x] JNI exporta 16 símbolos `Java_com_mcla_recomp_MainActivity_*` (verificado con `nm`)
- [x] CMake GLOB de los 120 `larecomp_recomp.N.cpp` integrado en `libmcla_main.so`
- [x] `libmcla_main.so` compilado: 34.6 MB (stripped, arm64-v8a)
- [x] **APK debug generado:** `m3-android/app/build/outputs/apk/debug/app-debug.apk` (55 MB)
- [x] Crash logger (`crash_logger.cpp`), config manager (`config.cpp`), menú (`menu.cpp`)
- [x] Gamepad manager (`gamepad_manager.cpp`)
- [x] Estructura de shader generator, vulkan catalog, updater y game hooks presente

**Gaps funcionales pendientes (ver ISSUES #009–#012):**
- [ ] `game_hooks.cpp`: 283 funciones hook son stubs (solo loguean "called - stub") — crítico para M4 (ISSUE #009)
- [ ] `updater.cpp`: parseo JSON sin implementar (stub)
- [ ] `shader_generator.cpp`: TODO — generación/escritura real de binario de shader
- [ ] `vulkan_catalog.cpp`: TODOs — instalación/desinstalación de drivers Turnip
- [ ] Rutas absolutas `/home/jhona/...` en CMake (portabilidad, ISSUE #010)
- [ ] Manifest exige GLES 3.0/AEP `required=true` en app Vulkan-only (ISSUE #011)
- [ ] Inconsistencia C++20 (`CMAKE_CXX_STANDARD`) vs C++23 (`cxx_std_23`) en CMake (ISSUE #012)
- [ ] Storage/ISO access (SAF + zero-copy fd) y audio AAudio: validación funcional en device

---

### ⏳ M4-M6 (PENDIENTES)
M4 es el próximo hito: boot test en hardware real. **Motorola Edge 50 Fusion conectado** y disponible para pruebas. Dependientes de cerrar gaps funcionales de M3. Ver PLAN.md para criterios de aceptación detallados.

---

## Bloqueos Resueltos

### ✅ Resuelto: XEX no disponible (ISSUE #008)
- **Estado:** RESUELTO (verificado 2026-11-20)
- **Resolución:** XEX proporcionado por el usuario en `/home/jhona/MCLA/default.xex` (9.2 MB, título `545407F8`); codegen ejecutado exitosamente sobre él
- **Fecha identificación:** 2026-11-19 · **Fecha resolución:** 2026-11-19/20

### ✅ Resuelto: libtinfo.so.5 faltante (ISSUE #007)
- **Estado:** RESUELTO / ya no bloquea (verificado 2026-11-20)
- **Verificación:** El binario host `rexglue-sdk/out/linux-amd64/rexglue` (v0.10.0) ejecuta correctamente sin `libtinfo.so.5` (`ldd` sin dependencias faltantes); el codegen se completó con él
- **Nota:** El sistema solo tiene `libtinfo.so.6`; un rebuild host desde cero con Clang 18 podría volver a requerir el workaround (symlink o `libtinfo5`)
- **Fecha identificación:** 2026-11-19

---

## Próximos Pasos

### Inmediatos (M3 → M4)
1. [ ] Implementar hooks críticos de `game_hooks.cpp` (283 stubs) — desbloquear boot funcional
2. [ ] Corregir manifest (GLES3 required en app Vulkan-only) y rutas absolutas de CMake
3. [ ] Unificar estándar C++ (C++23) en CMake de la app
4. [ ] Instalar `app-debug.apk` en Motorola Edge 50 Fusion y ejecutar boot test (M4)

### A corto plazo (M4)
5. [ ] Debug de errores de runtime en device (crash logger + logcat)
6. [ ] Implementar updater (parseo JSON) y shader generator real
7. [ ] Instalación de drivers Turnip vía vulkan_catalog
8. [ ] Validar funciones fallidas críticas del codegen y aplicar hacks LARecomp

### Medio plazo (M4→M5)
9. [ ] Tuning de rendimiento (fps target: 30+)
10. [ ] Implementar controles táctiles completos
11. [ ] AAudio integration testing
12. [ ] Thermal throttling mitigation

---

## Métricas de Avance

```
M0 ████████████████████████████████ 100% ✅
M1 ████████████████████████████████ 100% ✅
M2 ██████████████████████████████░░  95% 🔄
M3 ██████████████████████████░░░░░░  80% 🔄
M4 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0% ⏳
M5 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0% ⏳
M6 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0% ⏳
    0%        25%        50%        75%       100%
```

---

## Contacto del Equipo

- **Documentador (este rol):** Documentación, seguimiento, reporting
- **Coder:** Desarrollo técnico, builds, debugging
- **Control de Calidad:** Verificación de hitos
- **Marketing/Gráficos:** Assets visuales
- **Traductor:** Localización ES/EN

---

*Última actualización: 20 de noviembre de 2026 (auditoría técnica)*