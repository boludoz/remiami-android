plugins {
    id("com.android.application")
}

android {
    namespace = "com.revc"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.revc"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        ndk {
            // Solo arm64-v8a - NEON64 asm requiere esta arquitectura
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++14", "-fexceptions", "-frtti")
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DANDROID_TOOLCHAIN=clang",
                    // GL3 (default) or VULKAN: ./gradlew assembleRelease -PrevcRenderer=VULKAN
                    "-DREVC_RENDERER=" + (project.findProperty("revcRenderer") ?: "GL3")
                )
            }
        }
    }

    // CI signs with a fixed key from the repository secrets so updates install
    // over each other; locally, or without the secrets, the debug key is used.
    val ciKeystore = System.getenv("REVC_KEYSTORE")
    signingConfigs {
        if (ciKeystore != null && file(ciKeystore).exists()) {
            create("ci") {
                storeFile = file(ciKeystore)
                storePassword = System.getenv("REVC_KEYSTORE_PASSWORD")
                keyAlias = System.getenv("REVC_KEY_ALIAS")
                keyPassword = System.getenv("REVC_KEY_PASSWORD")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
            // release APKs must be signed to install; fall back to the debug key
            signingConfig = signingConfigs.findByName("ci") ?: signingConfigs.getByName("debug")
        }
        debug {
            isDebuggable = true
            isJniDebuggable = true
            signingConfigs.findByName("ci")?.let { signingConfig = it }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    sourceSets {
        getByName("main") {
            // Incluir assets del juego
            assets.srcDirs("src/main/assets", "../../gamefiles")
        }
    }

    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }

    ndkVersion = "26.1.10909125"

    lint {
        abortOnError = false
        checkReleaseBuilds = false
    }
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
}
