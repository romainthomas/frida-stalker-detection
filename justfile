set shell := ["bash", "-eu", "-o", "pipefail", "-c"]

root              := justfile_directory()
cmake_toolchains  := "/cmake"
linux_build_dir   := root + "/build/linux"
android_build_dir := root + "/build/android"
android_remote    := "/data/local/tmp/stalker-detect-android"
adb               := env_var_or_default("ADB", "adb")

[private]
default:
    @just --list --unsorted

build: build-linux build-android

# Build for Linux x86-64
build-linux:
    cmake -S "{{root}}/linux" -B "{{linux_build_dir}}" \
        -DCMAKE_TOOLCHAIN_FILE={{cmake_toolchains}}/linux/linux-x86_64-glibc-2.39.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build "{{linux_build_dir}}" --parallel

# Run the detection under Linux
run-linux: build-linux
    "{{linux_build_dir}}/stalker-detect-linux"

# Build for Android ARM64.
build-android:
    cmake -S "{{root}}/android" -B "{{android_build_dir}}" \
        -DCMAKE_TOOLCHAIN_FILE={{cmake_toolchains}}/android/toolchain-arm64.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build "{{android_build_dir}}" --parallel

# Run the detection under Android
run-android: build-android
    "{{adb}}" push "{{android_build_dir}}/stalker-detect-android" "{{android_remote}}"
    "{{adb}}" shell chmod 755 "{{android_remote}}"
    "{{adb}}" shell "{{android_remote}}"

# Build and run detection for both Android & Linux
run: run-linux run-android
