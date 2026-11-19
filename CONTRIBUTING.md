# Contributing to MCLA Recomp Android

Thank you for your interest in contributing to MCLA Recomp Android!

## How to Set Up the Development Environment

### Prerequisites

1. **Android NDK**: Version 26.1.10909125 or newer
   - Download from: https://developer.android.com/ndk/downloads/
   
2. **CMake**: Version 3.25 or newer
   - Can be downloaded from: https://cmake.org/download/
   
3. **Ninja**: Build system
   ```bash
   pip install ninja
   ```
   
4. **Java JDK**: Version 17 or newer
   - Download from: https://adoptium.net/
   
5. **Android SDK**: API 33+ (platform 33)
   - Can be installed via Android Studio or standalone
   
6. **Gradle**: Version 8+
   - Or use the Gradle wrapper included in the project

### Environment Variables

Set these environment variables before building:

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r26b
export ANDROID_SDK_HOME=/path/to/android-sdk
export PATH=$PATH:$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
```

### Build Steps

```bash
# Clone the repository
git clone <repo-url>
cd xbox-360-recomps

# Initialize git submodules (if any)
git submodule update --init --recursive

# Build the SDK first
./build_host_rexglue.sh

# Build the Android app
./gradlew assembleDebug
# or for release
./gradlew assembleRelease
```

## Project Structure

```
xbox-360-recomps/
├── README.md              # Project overview and documentation
├── LICENSE                # GPL-3.0 license
├── CHANGELOG.md           # Version history
├── PLAN.md                # Development roadmap
├── TIMELINE.md            # Project timeline
├── ISSUES.md              # Known issues
├── GLOSSARY.md            # Project terminology
├── patches/               # Custom patches for ReXGlue
│   ├── rexglue-v0.10.0-mcla.patch
│   └── ...
├── larecomp-ref/          # Reference LARecomp project (git submodule or copy)
├── rexglue-sdk/           # ReXGlue SDK source
├── rexglue-build-android/ # Android SDK build artifacts (gitignored)
├── rexglue-build-host/    # Host SDK build artifacts (gitignored)
├── m2-codegen/            # Code generation output
│   ├── generated/         # Generated C++ files (gitignored)
│   └── assets/default.xex   # MCLA XEX file (gitignored)
├── m3-android/            # Android application source
│   ├── app/
│   │   ├── jni/src/       # Native source code
│   │   └── src/main/      # Java/Kotlin source
│   └── build/             # Build artifacts (gitignored)
└── tools/                 # Development tools (gitignored)
```

## Coding Standards

- Use **C++17** for all native code
- Follow **Google C++ Style Guide** for naming conventions
- Use **snake_case** for functions and variables
- Use **PascalCase** for classes
- Document public APIs with Doxygen-style comments
- Keep functions under 100 lines when possible
- Use `constexpr` for compile-time constants

## Pull Request Process

1. Create a feature branch from `main`
2. Make your changes with clear commit messages
3. Run the build locally: `./gradlew assembleDebug`
4. Ensure no compiler warnings
5. Submit a pull request with a description of changes

## Issue Reporting

When reporting issues, please include:

1. Device model and Android version
2. NDK and SDK versions used
3. Full build log output
4. Steps to reproduce
5. Expected vs. actual behavior

## Testing

Before submitting a PR, test on:

1. Build compiles without warnings
2. APK installs and launches
3. Native libraries load correctly
4. XEX file is properly packaged

---

**Note**: This project requires a legally obtained copy of the original Xbox 360 game. We do not provide or support any illegal distribution methods.