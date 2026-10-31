cmake_minimum_required(VERSION 3.20)

project(stalker_detection_android LANGUAGES C)

set(FRIDA_GUM_ARCHIVE
    "${CMAKE_CURRENT_LIST_DIR}/../frida-gum-devkit-17.25.0-android-arm64.tar.xz"
    CACHE FILEPATH "Path to the Frida Gum Android ARM64 devkit")
set(FRIDA_GUM_DIR "${CMAKE_CURRENT_BINARY_DIR}/frida-gum")

if(NOT EXISTS "${FRIDA_GUM_ARCHIVE}")
  message(FATAL_ERROR "Frida Gum devkit not found: ${FRIDA_GUM_ARCHIVE}")
endif()

if(NOT EXISTS "${FRIDA_GUM_DIR}/libfrida-gum.a" OR
   NOT EXISTS "${FRIDA_GUM_DIR}/frida-gum.h")
  file(MAKE_DIRECTORY "${FRIDA_GUM_DIR}")
  file(ARCHIVE_EXTRACT INPUT "${FRIDA_GUM_ARCHIVE}"
       DESTINATION "${FRIDA_GUM_DIR}")
endif()

add_library(frida-gum STATIC IMPORTED)

set_target_properties(frida-gum PROPERTIES
  IMPORTED_LOCATION "${FRIDA_GUM_DIR}/libfrida-gum.a")

add_executable(stalker-detect-android main.c)

target_compile_definitions(stalker-detect-android PRIVATE ANDROID)

target_compile_options(stalker-detect-android PRIVATE
  -ffunction-sections -fdata-sections
  -Wl,--gc-sections
  -Wl,--strip-all
)

target_include_directories(stalker-detect-android PRIVATE "${FRIDA_GUM_DIR}")

target_link_libraries(stalker-detect-android PRIVATE
  frida-gum log m dl)
