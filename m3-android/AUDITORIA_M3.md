# Auditoría Técnica — M3 (App Android) — codegen → APK

Fecha: 2026-11-20
Estado general: **INTEGRACIÓN COMPLETA Y CONSTRUYENDO** (debug APK de 55 MB generado).

## 1. Verdict / Estado de integración

| Ítem | Estado |
|------|--------|
| Code C++ generado (m2-codegen/generated) | OK — 120 ficheros `larecomp_recomp.N.cpp` + `larecomp_init/register` |
| Compilación del código generado | OK — CMake `GLOB` los descubre y compila dentro de `libmcla_main.so` |
| JNI bridge (Java ↔ C++) | OK — `jni_bridge.cpp` exporta todos los símbolos; Java los declara |
| Config Gradle/CMake | OK — produce APK instalable válido |
| APK final | OK — `app/build/outputs/apk/debug/app-debug.apk` (55.070.380 bytes) |

**El APK ya empaqueta correctamente:**
- `lib/arm64-v8a/libmcla_main.so` (34,6 MB, stripped) — contiene el código generado recompilado
- `lib/arm64-v8a/librexgpu-xenos.so` (4,9 MB)
- `lib/arm64-v8a/librexruntime.so` (10,3 MB)
- `lib/arm64-v8a/libc++_shared.so`
- `assets/default.xex` (9,2 MB) + menú imágenes
- `libandroid/libdl/libm/liblog` son del sistema (no empaquetados) — correcto

## 2. Verificaciones realizadas (evidencia)

- `readelf -sW libmcla_main.so`: **todos** los símbolos JNI presentes
  (`Java_com_mcla_recomp_MainActivity_nativeInit/RunGame/Shutdown/OnSurface*/OnTouch/OnKey/SetAssetManager/SetNativeLibraryDir/...`).
- Símbolo `xstart` presente (WEAK GLOBAL, 1668 bytes) — entrada del juego enlazada.
- `NEEDED` de libmcla_main: librexgpu-xenos, librexruntime, libandroid, libdl, libm, liblog, libc++_shared — todos satisfechos en el APK.
- Entrada `xstart = 0x821322B8` coincide entre `jni_bridge.cpp`, `main.cpp` y `larecomp_register.cpp`.

## 3. Arquitectura (correcta)

- `MainActivity extends SDLActivity`; `main()` reemplazado a no-op (SDL no dlsymea `SDL_main`); el hilo de juego lo lanza `nativeInit()` (C++ `std::thread`).
- Ruta 100% **Vulkan** vía `rex::ui::SDLWindowedAppContext` + `WindowSDL` + `Presenter`/`ImmediateDrawer`. Sin OpenGL ES.
- El `SurfaceView` (MCLASurface) escribe ancho/alto por JNI; el hilo C++ espera la superficie antes de `Runtime::Setup`.
- Entrada/touch D-pad se inyectan como eventos SDL (`SDL_PushEvent`) en coordenadas mapeadas al render-target para ImGui.

## 4. Gaps críticos (funcionales)

1. **`game_hooks.cpp` dominado por stubs — EL GAP MÁS GRANDE.**
   - 283 ocurrencias `stubx` y 452 `(void)` en 42 KB. Las llamadas del juego a imports del kernel X360 (syscalls, DSP/XMA, periféricos) están mayoritariamente vacías/no-op.
   - **Riesgo:** el guest puede arrancar pero quedarse colgado o fallar al llegar a la primera syscall no implementada. Requiere validación en dispositivo real/logcat.

2. **`updater.cpp`:** función `Update()` no implementada (stub). El menú "Actualizar" no hará nada.

3. **`shader_generator.cpp` (1 TODO)** y **`vulkan_catalog.cpp` (2 TODO)** — casos de shader/feature Vulkan incompletos; pueden provocar fallos en drivers/GPU concretos.

## 5. Gaps de build config / robustez

1. **Rutas absolutas hardcodeadas en `app/jni/CMakeLists.txt`** (`/home/jhona/...` para REXSDK_DIR, GENERATED_DIR, libSDL3.a, runtime libs, XEX). Funciona solo en esta máquina; no reproducible en CI/otro host. Deberían ser variables/`-D` de Gradle o relativas a `rootProject`.
2. **Manifest inconsistente:** declara `uses-feature opengles.aep required=true` y `glEsVersion 0x00030000 required=true`, pero la app es Vulkan-only. En dispositivos sin GLES3 la instalación se bloquearía innecesariamente.
3. **Cruft de plantilla SDL en el manifest:** `android:enableVrMode="@string/vr_mode_enabled"` — vestigio del template SDLActivity, inofensivo pero a limpiar.
4. **Comentarios desactualizados:** `MCLASurface.java` afirma que "MainActivity NO es subclase de SDLActivity" (lo es); y habla de "publicar vía reflection" sin que haya reflection en el código. Engañoso para mantenimiento.
5. **Solo hay build Debug.** No existe `assembleRelease`/firma; el APK release no está configurado.

## 6. Acciones recomendadas (orden)
1. Validar en dispositivo: instalar `app-debug.apk`, revisar logcat (`MCLA_JNI`, `MCLA_Android`) hasta `Runtime::Setup` + `xstart`.
2. Completar los hooks de `game_hooks.cpp` necesarios para el arranque real del guest (es el cuello de botella funcional).
3. Parametrizar rutas en CMakeLists (eliminar absolutas).
4. Corregir el manifest (quitar requisito GLES3, quitar enableVrMode).
5. Implementar `updater.cpp` o desactivar la opción de menú.

## 7. Conclusión
La cadena **m2-codegen → CMake → JNI → APK** está **integrada y compilando**; el trabajo restante de M3 es **funcional (stubs de game_hooks/updater) + validación en hardware**, no de integración.