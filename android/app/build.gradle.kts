plugins {
    id("com.android.application")
}

android {
    namespace = "com.labsii.voices"
    compileSdk = 37

    defaultConfig {
        applicationId = "com.labsii.voices"
        minSdk = 29
        targetSdk = 37
        versionCode = 27
        versionName = "0.6.10"

        ndk {
            abiFilters += "arm64-v8a"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"))
        }
    }

    testOptions {
        unitTests.isReturnDefaultValues = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }

    androidResources {
        noCompress += listOf("bin", "cl")
    }

    sourceSets {
        getByName("main") {
            kotlin.directories.add("src/main/kotlin")
        }
    }
}

/**
 * Builds and stages the native executable the first time an APK is built.
 *
 * Keeping this in Gradle is important: Android Studio invokes Gradle directly,
 * so requiring a separate, undocumented terminal command makes a clean clone
 * impossible to build from the IDE.
 */
val stageKokoroRuntime = tasks.register<Exec>("stageKokoroRuntime") {
    val projectRoot = layout.projectDirectory.dir("../..").asFile
    val nativeProject = projectRoot.resolve("native/kokoro-82m")
    val setupScript = projectRoot.resolve("scripts/setup_deps.sh")
    val nativeBuildScript = nativeProject.resolve("scripts/build.sh")
    val stageScript = projectRoot.resolve("android/scripts/prepare_assets.sh")

    group = "build"
    description = "Builds and stages the Kokoro arm64 runtime for the APK."
    // Do not make CMake's generated build tree an input: it is created by
    // this task and would otherwise cause a needless second native build.
    inputs.files(fileTree(nativeProject) { exclude("build/**") })
    inputs.file(setupScript)
    inputs.file(nativeBuildScript)
    inputs.file(stageScript)
    outputs.dir(layout.projectDirectory.dir("src/main/jniLibs/arm64-v8a"))
    outputs.dir(layout.projectDirectory.dir("src/main/assets/kokoro"))

    workingDir = projectRoot
    commandLine(
        "bash", "-c",
        "./scripts/setup_deps.sh && ./native/kokoro-82m/scripts/build.sh --release && ./android/scripts/prepare_assets.sh"
    )
}

tasks.named("preBuild").configure {
    dependsOn(stageKokoroRuntime)
}

dependencies {
    implementation("com.google.android.material:material:1.14.0")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.11.0")
    testImplementation("junit:junit:4.13.2")
}
