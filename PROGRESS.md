# Progreso del Proyecto MCLA Recomp Android

Estado actual al 19 de noviembre de 2026 - Verificación de M2 (codegen)

## Resumen Ejecutivo

**Fase actual:** M2 — Codegen MCLA  
**Progreso general:** ~30% completado (M0 y M1 finalizados, M2 en curso)  
**Bloqueantes activos:** 2  
**Próximo objetivo:** Completar codegen y generar código C++ desde XEX

---

## Estado por Hito

| Hito | Nombre | Estado | Responsable | Fecha inicio | Fecha fin | % | Notas |
|------|--------|--------|-------------|--------------|-----------|---|-------|
| M0 | Entorno | ✅ Completado | Coder | 2026-11-18 | 2026-11-18 | 100% | Toolchain verificada |
| M1 | SDK ReXGlue ARM64 | ✅ Completado | Coder | 2026-11-18 | 2026-11-18 | 100% | librexruntime.so generado |
| **M2** | **Codegen MCLA** | **🔄 En progreso** | **Coder** | **2026-11-19** | **—** | **~30%** | **Submódulos listos, esperando XEX** |
| M3 | App Android | ⏳ Pendiente | Coder | — | — | 0% | Shell APK listo (m3-android/) |
| M4 | Boot en dispositivo | ⏳ Pendiente | Coder | — | — | 0% | Requiere M3 funcional |
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

### 🔄 M2 — Codegen MCLA (EN PROGRESO - 30%)
**Inicio:** 19 de noviembre de 2026  
**Estado parcial:** Estructura lista, generando código requiere XEX

**Completado:**
- [x] Submódulo `rexglue-sdk` clonado (v0.10.0)
- [x] Submódulo `larecomp-ref` clonado (referencia completa)
- [x] Configuración COBOL (`larecomp_config.toml`) preparada
- [x] Manifest MCLA (`mcla_manifest.toml`) leído
- [x] Scripts de debug validados (`debug_magic.py`, `gdb_break.gdb`)

**Pendiente:**
- [ ] Extraer XEX del ISO del juego (requiere intervención del usuario)
- [ ] Ejecutar `rexglue codegen` con XEX
- [ ] Generar ~250 funciones C++ desde PPC → ARM64
- [ ] Verificar funciones fallidas críticas
- [ ] Aplicar hacks de LARecomp/mcla-recomp al código generado

**Bloqueantes:**
1. 🔴 **CRÍTICO:** No se cuenta con archivo XEX del juego (Midnight Club: Los Angeles)
   - El XEX debe ser extraído de un disco físico comprado por el usuario
   - Título Xbox 360: `545407F8`
   - Ver ISSUE #008

2. 🟡 **MEDIO:** Dependencia `libtinfo.so.5` faltante para Clang 18
   - Afecta build del SDK en host Linux
   - Workaround disponible o instalar paquete `libtinfo-compat`
   - Ver ISSUE #007

---

### ⏳ M3 — App Android (PENDIENTE)
**Preparado:** Shell APK en `m3-android/`

Componentes listos para implementar cuando M2 esté completo:
- Mainactivity.java
- JNI bridge (jni_bridge.cpp)
- Game hooks (game_hooks.cpp)
- Shader generator (shader_generator.cpp)
- Crash logger (crash_logger.cpp)
- Config manager (config.cpp)
- Input/gamepad manager
- Storage/ISO access (SAF + zero-copy fd)
- Audio AAudio nativo

---

### ⏳ M4-M6 (PENDIENTES)
Dependientes de hitos previos. Ver PLAN.md para criterios de aceptación detallados.

---

## Bloqueos Activos

### 🔴 Bloqueo Crítico: XEX no disponible
- **Impacto:** Imposible continuar M2 sin fuente de juego
- **Solución requerida:** Usuario debe proporcionar XEX extraído de disco físico
- **Título Xbox:** `545407F8` (Midnight Club: Los Angeles Complete Edition)
- **Referencia:** ISSUE #008
- **Fecha identificación:** 2026-11-19

### 🟡 Bloqueo Medio: libtinfo.so.5 faltante
- **Impacto:** Builds del SDK en host pueden fallar al linker con Clang 18
- **Workaround:** Instalar `libtinfo-compat` o usar Clang 17 para host
- **Referencia:** ISSUE #007
- **Fecha identificación:** 2026-11-19

---

## Próximos Pasos

### Inmediatos ( blockers desbloqueados )
1. [ ] Obtener XEX del juego de usuario → desbloquear M2
2. [ ] Resolver libtinfo.so.5 (instalar paquetes o workaround)
3. [ ] Ejecutar codegen completo sobre XEX
4. [ ] Validar integración de código generado con app Android

### A corto plazo (post-M2)
5. [ ] Integrar funciones generadas en m3-android
6. [ ] Construir APK debug
7. [ ] Probar boot en emulador/dispositivo
8. [ ] Debug初期de errores de runtime

### Medio plazo (M3→M5)
9. [ ] Tuning de rendimiento (fps target: 30+)
10. [ ] Implementar controles táctiles completos
11. [ ] AAudio integration testing
12. [ ] Thermal throttling mitigation

---

## Métricas de Avance

```
M0 ████████████████████████████████ 100% ✅
M1 ████████████████████████████████ 100% ✅
M2 ██████████░░░░░░░░░░░░░░░░░░░░░░  30% 🔄
M3 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0% ⏳
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

*Última actualización: 19 de noviembre de 2026*
