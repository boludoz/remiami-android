#!/bin/bash
# Script to compile reVC for Android using Gradle Wrapper

echo "======================================"
echo "  reVC Android Build Script"
echo "======================================"
echo

# Go to the Android project directory
cd "$(dirname "$0")"

# Setup Android SDK path (local to project if not set globally)
if [ -z "$ANDROID_SDK_ROOT" ]; then
    if [ "$(uname)" = "Darwin" ]; then
        # macOS
        ANDROID_SDK_ROOT="$HOME/Library/Android/sdk"
    else
        # Linux
        ANDROID_SDK_ROOT="$HOME/Android/Sdk"
    fi
    echo "Using default Android SDK location: $ANDROID_SDK_ROOT"
else
    echo "Using Android SDK: $ANDROID_SDK_ROOT"
fi

export ANDROID_HOME="$ANDROID_SDK_ROOT"

# Create licenses directory and accept licenses automatically
mkdir -p "$ANDROID_SDK_ROOT/licenses"

echo "Accepting Android SDK licenses..."

echo -e "\n24333f8a63b6825ea9c5514f83c2829b004d1fee" > "$ANDROID_SDK_ROOT/licenses/android-sdk-license"
echo -e "\n84831b9409646a918e30573bab4c9c91346d8abd" > "$ANDROID_SDK_ROOT/licenses/android-sdk-preview-license"
echo -e "\nd975f751698a77b662f1254ddbeed3901e976f5a" > "$ANDROID_SDK_ROOT/licenses/intel-android-extra-license"
echo -e "\n33b6a2b64607f11b759f320ef9dff4ae5c47d97a" > "$ANDROID_SDK_ROOT/licenses/google-gdk-license"
echo -e "\ne9acab5b5fbb560a72cfaecce8946896ff6aab9d" > "$ANDROID_SDK_ROOT/licenses/mips-android-sysimage-license"

echo "Licenses accepted successfully."
echo

# Download gradle-wrapper.jar if it doesn't exist
if [ ! -f "gradle/wrapper/gradle-wrapper.jar" ]; then
    echo "Downloading Gradle Wrapper..."
    mkdir -p gradle/wrapper
    curl -L -o gradle/wrapper/gradle-wrapper.jar https://raw.githubusercontent.com/gradle/gradle/master/gradle/wrapper/gradle-wrapper.jar
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to download Gradle Wrapper"
        echo "Please download it manually or use a system with Gradle installed"
        exit 1
    fi
fi

# Check if gradlew exists
if [ ! -f "gradlew" ]; then
    echo "ERROR: gradlew not found"
    echo "Please ensure the Gradle Wrapper is properly set up"
    exit 1
fi

# Make gradlew executable
chmod +x gradlew

# Check argument
if [ -z "$1" ]; then
    echo "Usage: build_android.sh [debug|release|clean]"
    echo
    echo "Commands:"
    echo "  debug   - Build debug version"
    echo "  release - Build release version"
    echo "  clean   - Clean build files"
    echo
    BUILD_TYPE="debug"
else
    BUILD_TYPE="$1"
fi

if [ "$BUILD_TYPE" = "clean" ]; then
    echo "Cleaning project..."
    ./gradlew clean
elif [ "$BUILD_TYPE" = "release" ]; then
    echo "Building RELEASE version..."
    ./gradlew assembleRelease
else
    echo "Building DEBUG version..."
    ./gradlew assembleDebug
fi

if [ $? -eq 0 ]; then
    echo
    echo "======================================"
    echo "  Build successful!"
    echo "======================================"
    echo
    if [ "$BUILD_TYPE" = "release" ]; then
        echo "APK generated at: app/build/outputs/apk/release/"
    else
        echo "APK generated at: app/build/outputs/apk/debug/"
    fi
else
    echo
    echo "======================================"
    echo "  Build failed"
    echo "======================================"
fi
