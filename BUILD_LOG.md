# Registro de Builds

Historial de intentos de build, configuraciones y resultados.

---

## Toolchain Utilizada

### Para Android (ARM64)
| Componente | Versión | Ruta |
|------------|---------|------|
| NDK | 26.1.10909125 | `$ANDROID_NDK_HOME` |
| Clang (target) | 17.0.2 | NDK toolchains/llvm/prebuilt/linux-x86_64/bin/clang |
| CMake | 3.31.0 | `/home/jhon/tools/cmake-3.31.0-linux-x86_64/bin/cmake` |
| Ninja | 1.12.1 | `/home/jhon/tools/ninja` |

### Para Host (Linux x86_64)
| Componente | Versión | Ruta |
|------------|---------|------|
| Clang | 18.1.8 | `/home/jhon/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin/clang` |
| CMake | 3.31.0 | `/home/jhon/tools/cmake-3.31.0-linux-x86_64/bin/cmake` |
| Ninja | 1.12.1 | `/home/jhon/tools/ninja` |

---

## Builds Realizados

### Build #001 — ReXGlue SDK Android ARM64
**Fecha:** 18 de noviembre de 2026  
**Hito:** M1  
**Estado:** ✅ EXITOSO

**Comando ejecutado:**
```bash
cd rexglue-build-android
cmake -B build -S .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24 \
  -GNinja
cmake --build build -- -j$(nproc)
```

**Resultados:**
- Librería generada: `librexruntime.so` (86.5 MB)
- Compatibilidad: Android API 24+ (Android 7.0+)
- Vulkan habilitado ✅
- SDL3 stubs incluidos ✅

**Notas:** Build limpio, sin warnings críticos. Patch aplicado correctamente.

---

### Build #002 — ReXGlue SDK Host Linux x86_64
**Fecha:** 19 de noviembre de 2026  
**Hito:** M2 (preparación codegen)  
**Estado:** ⚠️ PARCIALMENTE EXITOSO / BLOQUEADO

**Comando ejecutado (headless mode):**
```bash
cd rexglue-build-host
rm -rf ./*
cmake -GNinja \
  -DCMAKE_MAKE_PROGRAM=/home/jhon/tools/ninja \
  -DCMAKE_C_COMPILER=/home/jhon/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin/clang \
  -DCMAKE_CXX_COMPILER=/home/jhon/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin/clang++ \
  -DCMAKE_C_FLAGS="-march=x86-64-v2 -I/home/jhon/tools/local-libs/include" \
  -DCMAKE_CXX_FLAGS="-march=x86-64-v2 -stdlib=libc++ -I/home/jhon/tools/local-libs/include" \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
  -DCMAKE_BUILD_TYPE=Release \
  -DREXGLUE_BUILD_TESTS=OFF \
  -DREXGLUE_ENABLE_FIDELITYFX=OFF \
  -DREXGLUE_ENABLE_PERF_COUNTERS=ON \
  -DREXGLUE_ENABLE_SANITIZERS=OFF \
  -DREXGLUE_ENABLE_TRACY=OFF \
  -DREXGLUE_USE_VULKAN=ON \
  /home/jhon/xbox-recomp/rexglue-sdk
ninja -j$(nproc) rexglue
```

**Resultados:**
- Configuración CMake: ✅ OK
- Compilación: ⚠️ Falló en fase de linking
- Error principal:
  ```
  ld: error: unable to find library -ltinfo
  ld: error: cannot find libtinfo.so.5
  ```
- Binarios parciales: `rexcodegen` (parcial), `rexruntime` (no incluido en host)

**Análisis del error:**
Clang 18.1.8 busca `libtinfo.so.5` (ncurses legacy) pero el sistema tiene `libtinfo.so.6`.

**Workarounds identificados:**
1. Instalar `libtinfo5` desde apt
2. Crear symlink: `ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5`
3. Cambiar toolchain host a Clang 17.x

**Referencia:** ISSUE #007

---

### Build #003 — Intento de Codegen M2
**Fecha:** 19 de noviembre de 2026  
**Hito:** M2  
**Estado:** ❌ NO EJECUTADO (bloqueado)

**Motivo:** Ausencia de archivo XEX del juego.

**Comando planeado:**
```bash
export PATH="$ROOT/tools/cmake-3.31.0-linux-x86_64/bin:$ROOT/tools/ninja:$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH"

# Paso 1: Build host tools (después de resolver libtinfo)
$ROOT/tools/ninja -j$(nproc) rexcodegen

# Paso 2: Codegen
$ROOT/rexglue-build-host/rexcodegen \
  --manifest $ROOT/m2-codegen/mcla_manifest.toml \
  --config $ROOT/m2-codegen/larecomp_config.toml \
  --output $ROOT/m2-codegen/generated/ \
  /ruta/al/XEX/MCLA.xex
```

**Bloqueo:** No se dispone de `MCLA.xex` o equivalente.

**Referencia:** ISSUE #008

---

## Resumen de Builds

| Build | Fecha | Objetivo | Estado | Duración est. |
|-------|-------|----------|--------|---------------|
| #001 | 2026-11-18 | ReXGlue SDK Android | ✅ Éxito | ~3-5 min |
| #002 | 2026-11-19 | ReXGlue SDK Host | ⚠️ Parcial | ~2 min (falla en link) |
| #003 | 2026-11-19 | Codegen M2 | ❌ Bloqueado | — |

---

## Errores Comunes y Soluciones

### Error: "cannot find libtinfo.so.5"
**Causa:** Sistema moderno usa libtinfo.so.6 (ncurses 6.x)  
**Solución rápida:**
```bash
sudo ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
```

**Solución permanente:** Instalar paquete compat
```bash
sudo apt install libtinfo5
```

---

### Error: "NDK platform android-24 not found"
**Causa:** NDK no configurado correctamente  
**Solución:**
```bash
export ANDROID_NDK_HOME=/ruta/a/android-ndk-r26b
export PATH=$PATH:$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
```

---

## Archivos de Build Generados

### SDK Android
```
rexglue-build-android/build/librexruntime.so    # 86.5 MB
rexglue-build-android/build/include/            # Headers públicos
```

### SDK Host (parcial)
```
rexglue-build-host/CMakeCache.txt               # Configuración CMake
rexglue-build-host/rexcodegen                   # Binario parcial (link fallido)
```

---

## Próximos Builds Planeados

1. **Build #002b** — Reintentar build host tras resolver libtinfo
2. **Build #004** — Ejecutar codegen cuando se tenga XEX
3. **Build #005** — Construir APK debug con código generado
4. **Build #006** — Construir APK release para test en dispositivo

---

*Última actualización: 19 de noviembre de 2026*
