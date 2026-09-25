@echo off
REM Script to compile reVC for Android using Gradle Wrapper

echo ======================================
echo   reVC Android Build Script
echo ======================================
echo.

REM Go to the Android project directory
cd /d "%~dp0"

REM Setup Android SDK path (local to project if not set globally)
if "%ANDROID_SDK_ROOT%"=="" (
    set "ANDROID_SDK_ROOT=%LOCALAPPDATA%\Android\Sdk"
    echo Using default Android SDK location: %ANDROID_SDK_ROOT%
) else (
    echo Using Android SDK: %ANDROID_SDK_ROOT%
)

set ANDROID_HOME=%ANDROID_SDK_ROOT%

REM Create licenses directory and accept licenses automatically
if not exist "%ANDROID_SDK_ROOT%\licenses" mkdir "%ANDROID_SDK_ROOT%\licenses"

echo Accepting Android SDK licenses...
(
echo.
echo 24333f8a63b6825ea9c5514f83c2829b004d1fee
) > "%ANDROID_SDK_ROOT%\licenses\android-sdk-license"

(
echo.
echo 84831b9409646a918e30573bab4c9c91346d8abd
) > "%ANDROID_SDK_ROOT%\licenses\android-sdk-preview-license"

(
echo.
echo d975f751698a77b662f1254ddbeed3901e976f5a
) > "%ANDROID_SDK_ROOT%\licenses\intel-android-extra-license"

(
echo.
echo 33b6a2b64607f11b759f320ef9dff4ae5c47d97a
) > "%ANDROID_SDK_ROOT%\licenses\google-gdk-license"

(
echo.
echo e9acab5b5fbb560a72cfaecce8946896ff6aab9d
) > "%ANDROID_SDK_ROOT%\licenses\mips-android-sysimage-license"

echo Licenses accepted successfully.
echo.

REM Download gradle-wrapper.jar if it doesn't exist
if not exist "gradle\wrapper\gradle-wrapper.jar" (
    echo Downloading Gradle Wrapper...
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/gradle/gradle/master/gradle/wrapper/gradle-wrapper.jar' -OutFile 'gradle\wrapper\gradle-wrapper.jar'}"
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to download Gradle Wrapper
        echo Please download it manually or use a system with Gradle installed
        goto :end
    )
)

REM Check if gradlew.bat exists
if not exist "gradlew.bat" (
    echo ERROR: gradlew.bat not found
    echo Please ensure the Gradle Wrapper is properly set up
    goto :end
)

REM Check argument
if "%1"=="" (
    echo Usage: build_android.bat [debug^|release^|clean^|install]
    echo.
    echo Commands:
    echo   debug   - Build debug version
    echo   release - Build release version
    echo   clean   - Clean build files
    echo   install - Install debug APK to connected device
    echo.
    set BUILD_TYPE=debug
) else (
    set BUILD_TYPE=%1
)

if "%BUILD_TYPE%"=="clean" (
    echo Cleaning project...
    call gradlew.bat clean
    goto :end
)

if "%BUILD_TYPE%"=="install" (
    echo Installing DEBUG APK to connected device...
    call gradlew.bat installDebug
    if %ERRORLEVEL% EQU 0 (
        echo APK installed successfully.
    ) else (
        echo Failed to install APK.
    )
    goto :end
)

if "%BUILD_TYPE%"=="release" (
    echo Building RELEASE version...
    call gradlew.bat assembleRelease
) else (
    echo Building DEBUG version...
    call gradlew.bat assembleDebug
)

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ======================================
    echo   Build successful!
    echo ======================================
    echo.
    if "%BUILD_TYPE%"=="release" (
        echo APK generated at: app\build\outputs\apk\release\
    ) else (
        echo APK generated at: app\build\outputs\apk\debug\
    )
) else (
    echo.
    echo ======================================
    echo   Build failed
    echo ======================================
)

:end
pause
