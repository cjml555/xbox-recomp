# Contribuir a MCLA Recomp Android

¡Gracias por tu interés en contribuir a MCLA Recomp Android!

## Cómo configurar el entorno de desarrollo

### Requisitos previos

1. **Android NDK**: Versión 26.1.10909125 o posterior
   - Descargar desde: https://developer.android.com/ndk/downloads/
   
2. **CMake**: Versión 3.25 o posterior
   - Se puede descargar desde: https://cmake.org/download/
   
3. **Ninja**: Sistema de build
   ```bash
   pip install ninja
   ```
   
4. **Java JDK**: Versión 17 o posterior
   - Descargar desde: https://adoptium.net/
   
5. **Android SDK**: API 33+ (platform 33)
   - Se puede instalar vía Android Studio o de forma independiente
   
6. **Gradle**: Versión 8+
   - O usar el wrapper de Gradle incluido en el proyecto

### Variables de entorno

Configura estas variables de entorno antes de construir:

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26b
export ANDROID_SDK_HOME=/path/to/android-sdk
export PATH=$PATH:$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
```

### Pasos de construcción

```bash
# Clonar el repositorio
git clone <repo-url>
cd xbox-360-recomps

# Inicializar submódulos git (si existen)
git submodule update --init --recursive

# Construir primero el SDK
./build_host_rexglue.sh

# Construir la app Android
./gradlew assembleDebug
# o para release
./gradlew assembleRelease
```

## Estructura del proyecto

```
xbox-360-recomps/
├── README.md              # Visión general y documentación del proyecto
├── LICENSE                # Licencia GPL-3.0
├── CHANGELOG.md           # Historial de versiones
├── PLAN.md                # Hoja de ruta del desarrollo
├── TIMELINE.md            # Línea de tiempo del proyecto
├── ISSUES.md              # Issues conocidas
├── GLOSSARY.md            # Terminología del proyecto
├── patches/               # Parches personalizados para ReXGlue
│   ├── rexglue-v0.10.0-mcla.patch
│   └── ...
├── larecomp-ref/          # Proyecto de referencia LARecomp (submódulo git o copia)
├── rexglue-sdk/           # Código fuente del SDK ReXGlue
├── rexglue-build-android/ # Artefactos de construcción del SDK Android (gitignored)
├── rexglue-build-host/    # Artefactos de construcción del SDK host (gitignored)
├── m2-codegen/            # Salida de generación de código
│   ├── generated/         # Archivos C++ generados (gitignored)
│   └── assets/default.xex   # Archivo XEX de MCLA (gitignored)
├── m3-android/            # Fuente de la aplicación Android
│   ├── app/
│   │   ├── jni/src/       # Código fuente nativo
│   │   └── src/main/      # Fuente Java/Kotlin
│   └── build/             # Artefactos de construcción (gitignored)
└── tools/                 # Herramientas de desarrollo (gitignored)
```

## Estándares de codificación

- Usar **C++17** para todo el código nativo
- Seguir la **Guía de Estilo Google C++** para convenciones de nombres
- Usar **snake_case** para funciones y variables
- Usar **PascalCase** para clases
- Documentar APIs públicas con comentarios estilo Doxygen
- Mantener funciones bajo 100 líneas cuando sea posible
- Usar `constexpr` para constantes de compilación

## Proceso de Pull Request

1. Crear una rama de características desde `main`
2. Realizar tus cambios con mensajes de commit claros
3. Ejecutar el build localmente: `./gradlew assembleDebug`
4. Asegurarse de que no hay warnings del compilador
5. Enviar un pull request con una descripción de los cambios

## Reporte de Issues

Al reportar issues, por favor incluye:

1. Modelo de dispositivo y versión de Android
2. Versiones de NDK y SDK utilizadas
3. Log completo de build
4. Pasos para reproducir
5. Comportamiento esperado vs. real

## Testing

Antes de enviar un PR, prueba en:

1. Build compila sin warnings
2. APK instala y lanza correctamente
3. Bibliotecas nativas cargan correctamente
4. Archivo XEX está empaquetado apropiadamente

---

**Nota:** Este proyecto requiere una copia legal del juego original de Xbox 360. No proporcionamos ni apoyamos métodos de distribución ilegales.
