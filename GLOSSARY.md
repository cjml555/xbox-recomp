# Glosario Técnico / Technical Glossary

Diccionario español-inglés para coherencia en documentación, comentarios de código y comunicaciones del equipo de recompilación Xbox 360.

---

## Recompilación / Recompilation

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| recompilación estática | static recompilation | Traducción de binarios PPC a arm64 en tiempo de compilación | Proceso principal de conversión de XEX |
| generación de código | codegen | Fase que produce código arm64 a partir de PPC analizado | Pipeline del recompilador |
| código huésped | host code | Código nativo que se ejecuta en el dispositivo Android | Contraste con código invitado |
| código invitado | guest code | Código original de Xbox 360 (PPC) siendo emulado/recompilado | Binarios XEX originales |
| PPC / PowerPC | PPC / PowerPC | Arquitectura CPU de Xbox 360 (big-endian) | Arquitectura fuente |
| arm64 / AArch64 | arm64 / AArch64 | Arquitectura CPU destino (Android moderno) | Arquitectura objetivo |
| emulación de capa alta | HLE (High-Level Emulation) | Simulación de APIs del SO en vez de instrucciones bajas | Xam, XGraphics, audio |
| entrada/salida mapeada en memoria | MMIO (Memory-Mapped I/O) | Registro de hardware accesible como memoria | GPU, audio, input |
| vectorización VMX | VMX / Altivec | Instrucciones vectoriales de PowerPC | SIMD de Xbox 360 |
| traducción SIMD vía SIMDe | SIMDe translation | Emulación de vectores usando la librería SIMDe | Puente PPC → ARM NEON/SVE |
| endianidad grande | big-endian | Orden de bytes de Xbox 360 (PPC) | Conversión requerida a little-endian |
| endianidad pequeña | little-endian | Orden de bytes de Android/arm64 | Formato nativo del destino |
| thunk de función | function thunk | Función puente entre código recompilado y runtime | Llamadas a sistema/emuladas |
| análisis de flujo de control | control flow analysis | Detección de bloques básicos y saltos | Fase inicial del recompilador |
| tabla de saltos | jump table | Estructura de saltos indirectos calculados | Switch recompilado |

---

## ReXGlue

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| manifiesto | manifest | Archivo que describe módulos, dependencias y símbolos del juego | `manifest.json` / `.yaml` |
| configuración | config | Parámetros de runtime y hooks específicos por título | `config.ini` / `.toml` |
| gancho de ensamblador intermedio | mid-asm hook | Inyección de código personalizado en puntos específicos del binario | Parches de compatibilidad |
| pista de función | function hint | Metadato que ayuda al recompilador a identificar límites de funciones | Mejora precisión del análisis |
| tiempo de ejecución | runtime | Librerías que proveen APIs emuladas en tiempo de ejecución | `librexglue.so` |
| rutina stub | stub | Función vacía o mínima que satisface enlazado temporal | Símbolos no implementados |
| intervalo vertical | vblank | Señal de refresco de pantalla, usada para sincronización | Timing de emulación |
| colapso de vblank | vblank collapse | Múltiples esperas de vblank combinadas para evitar stutter | Optimización de timing |
| tabla de símbolos | symbol table | Mapeo de direcciones a nombres de funciones/variables | Debug y hooks |
| parche de memoria | memory patch | Modificación de datos en memoria en tiempo de ejecución | Cheats, fixes |
| redirección de llamada | call redirect | Desvío de llamada de función a implementación alternativa | HLE hooks |
| inicialización diferida | lazy initialization | Carga de recursos bajo demanda | Optimización de arranque |

---

## Android / NDK

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| ABI arm64 | arm64-v8a | Arquitectura binaria de Android moderno | Target principal |
| cadena de herramientas | toolchain | Compilador, enlazador y utilidades cruzadas | `aarch64-linux-android-clang` |
| raíz del sistema | sysroot | Headers y librerías del sistema destino | NDK platforms |
| interfaz nativa de Java | JNI (Java Native Interface) | Puente entre código nativo C++ y Java/Kotlin | Integración con app Android |
| audio de baja latencia | AAudio | API de audio nativo de Android | Emulación de XAudio |
| framework de acceso a almacenamiento | SAF (Storage Access Framework) | API de Android para acceso a archivos | Selección de XEX/ISO |
| controlador Vulkan de código abierto | Turnip | Driver Vulkan para GPU Adreno (Mesa) | Renderizado en Android |
| proyecto Mesa | Mesa | Implementación open-source de gráficos (OpenGL/Vulkan) | Stack gráfico Linux/Android |
| API de gráficos de bajo nivel | Vulkan | API gráfica moderna usada por el backend | Reemplazo de Xenos D3D |
| capa de compatibilidad | compatibility layer | Traducción de llamadas D3D9 a Vulkan | Xenos → Vulkan |
| gestor de memoria ION | ION memory allocator | Sistema de asignación de memoria en Android | Buffers GPU |
| servicio de gráficos SurfaceFlinger | SurfaceFlinger | Compositor de ventanas de Android | Presentación de frames |

---

## Gráficos / Graphics

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| memoria de video embebida | eDRAM | 10 MB de VRAM integrada en Xenos | Render targets, depth buffers |
| renderizado por mosaicos | tile rendering | Técnica de renderizado que divide la pantalla en tiles | Arquitectura de Xenos |
| GPU Xenos | Xenos | GPU personalizada de Xbox 360 (ATI/AMD) | Hardware fuente |
| alfa difuminado | dithered alpha | Técnica de transparencia con patrón de puntos | Efectos de partículas |
| cadena de intercambio | swapchain | Secuencia de buffers para presentación | Vulkan/D3D swap buffers |
| caché de pipeline | pipeline cache | Almacenamiento de pipelines de shaders compilados | Reduce stutter en replays |
| sombreador unificado | unified shader | Arquitectura de shaders sin separación vertex/pixel | Xenos shader model 3.0 |
| buffer de marco | framebuffer | Buffer que contiene la imagen renderizada | Composición final |
| buffer de profundidad | depth buffer / Z-buffer | Buffer que almacena información de profundidad | Test de profundidad |
| muestreo anisotrópico | anisotropic filtering | Filtrado de texturas en ángulos oblicuos | Calidad de texturas |
| oclusión de entorno | ambient occlusion | Técnica de sombreado para oclusión ambiental | Efectos de iluminación |
| anti-aliasing temporal | TAA (Temporal Anti-Aliasing) | AA que usa información de frames anteriores | Suavizado de bordes |

---

## Build / Compilación

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| preajuste de CMake | CMake preset | Configuración predefinida de build en `CMakePresets.json` | `android-arm64-release` |
| sistema de build Ninja | Ninja | Sistema de build rápido usado por CMake | Generador predeterminado |
| compilador Clang | Clang | Compilador LLVM para C/C++ | Toolchain NDK |
| enlazador LLD | LLD | Enlazador rápido de LLVM | Reemplazo de GNU ld |
| sistema de build de Android | Gradle | Sistema de build para proyectos Android | `build.gradle` |
| paquete de Android | APK (Android Package) | Formato de distribución de apps Android | `app-release.apk` |
| bundle de Android | AAB (Android App Bundle) | Formato de publicación en Play Store | Distribución optimizada |
| variante de build | build variant | Combinación de flavor + buildType en Gradle | `armeabiRelease`, `arm64Debug` |
| firma de aplicación | app signing | Proceso de firmar APK con clave | Distribución |
| ofuscación de código | code obfuscation | Transformación de código para dificultar ingeniería inversa | R8/ProGuard |
| árbol de salida | output tree | Estructura de directorios de build | `build/intermediates` |
| dependencias transitivas | transitive dependencies | Dependencias de dependencias | Gradle resolution |

---

## Testing / Pruebas

| ES | EN | Definición breve | Contexto |
|----|----|------------------|----------|
| limitación térmica | thermal throttling | Reducción de rendimiento por temperatura | GPU/CPU throttling |
| tartamudeo de shaders | shader stutter | Pausas mientras se compilan shaders en runtime | First-time compilation |
| ritmo de frames | frame pacing | Distribución uniforme del tiempo entre frames | Smoothness |
| colapso de vblank | vblank collapse | Múltiples waits combinados en uno solo | Timing fix |
| caída de frames | frame drop | Frame que no se presenta a tiempo | Performance issue |
| tiempo de frame | frame time | Duración de un frame en ms | 16.67ms = 60 FPS |
| presupuesto de tiempo | time budget | Tiempo máximo asignado por frame | 16ms para 60Hz |
| análisis de rendimiento | profiling | Medición de tiempos de CPU/GPU | Perfetto, Systrace |
| fuga de memoria | memory leak | Memoria asignada no liberada | Long session testing |
| condición de carrera | race condition | Bug por timing de hilos concurrentes | Audio, input threads |
| regresión de rendimiento | performance regression | Degradación vs versión anterior | CI benchmarks |
| prueba de estrés | stress test | Ejecución prolongada para detectar inestabilidad | Thermal, memory tests |

---

## Uso / Usage

Este glosario debe consultarse para:

1. **Documentación**: Mantener consistencia en términos técnicos
2. **Comentarios de código**: Usar términos estandarizados en inglés con equivalente español si necesario
3. **Comunicación del equipo**: Evitar ambigüedades en discusiones técnicas
4. **Mensajes de error**: Redactar claros y consistentes

### Convenciones

- En **código**: usar términos en inglés (`vblank`, `stub`, `hook`)
- En **documentación en español**: incluir equivalente inglés la primera vez
- En **issues/PRs**: título en inglés, descripción puede ser bilingüe

---

*Última actualización: 2026-11-18*
