# Building reVC for Android

## Requirements

- **Java 17+ SDK** - [Download here](https://www.oracle.com/java/technologies/downloads/)

That's it! Everything else downloads automatically.

## Build

**Windows:**
```cmd
build_android.bat debug
```

**Linux/macOS:**
```bash
chmod +x build_android.sh gradlew
./build_android.sh debug
```

## Output

APK location: `app/build/outputs/apk/debug/app-debug.apk`

## Install

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## Game Files

Copy GTA Vice City files to:
```
/storage/emulated/0/reVC/
```

## Notes

- First build takes longer (downloads SDK, NDK, dependencies)
- Use `release` instead of `debug` for release build
- Use `clean` to clean build files
