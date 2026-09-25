cmake_minimum_required(VERSION 3.22.1)

project("revc" LANGUAGES C CXX)

set(EXECUTABLE revc)

# C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================================
# Deshabilitar TODAS las warnings
# ============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

# Rutas del proyecto re3
get_filename_component(REVC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(REVC_SRC_DIR "${REVC_ROOT}/src")
set(REVC_VENDOR_DIR "${REVC_ROOT}/vendor")

message(STATUS "REVC_ROOT: ${REVC_ROOT}")
message(STATUS "REVC_SRC_DIR: ${REVC_SRC_DIR}")

if(NOT EXISTS "${REVC_SRC_DIR}")
    message(FATAL_ERROR "Source directory not found: ${REVC_SRC_DIR}")
endif()

# ============================================================================
# Renderer: GL3 (GLES 3 through SDL3, supports every graphics option: Neo
# pipelines, NEW_RENDERER, screen droplets) or VULKAN. Pick it with
#   ./gradlew assembleRelease -PrevcRenderer=VULKAN
# ============================================================================
set(REVC_RENDERER "GL3" CACHE STRING "Renderer backend: GL3 or VULKAN")
set_property(CACHE REVC_RENDERER PROPERTY STRINGS GL3 VULKAN)
if(NOT REVC_RENDERER STREQUAL "GL3" AND NOT REVC_RENDERER STREQUAL "VULKAN")
    message(FATAL_ERROR "REVC_RENDERER must be GL3 or VULKAN, got '${REVC_RENDERER}'")
endif()
message(STATUS "reVC renderer: ${REVC_RENDERER}")

# ============================================================================
# Módulo CMake personalizado para Android (FindOpenGL -> GLES3)
# ============================================================================
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ============================================================================
# Submodules - Usar dependencias locales en vendor/
# ============================================================================

# --- SDL3 ---
message(STATUS "Configuring SDL3 from submodule...")
set(SDL3_DIR "${REVC_VENDOR_DIR}/SDL3")
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)

# SDL3 Subsystems (Enable as needed)
set(SDL_AUDIO ON CACHE BOOL "" FORCE)
set(SDL_VIDEO ON CACHE BOOL "" FORCE)
set(SDL_RENDER ON CACHE BOOL "" FORCE)
set(SDL_EVENTS ON CACHE BOOL "" FORCE)
set(SDL_JOYSTICK ON CACHE BOOL "" FORCE)
set(SDL_HAPTIC ON CACHE BOOL "" FORCE)
set(SDL_HIDAPI ON CACHE BOOL "" FORCE)
set(SDL_POWER ON CACHE BOOL "" FORCE)
# SDL3 usually auto-detects threads/timers, but we can force if needed
# set(SDL_THREADS ON CACHE BOOL "" FORCE) 

add_subdirectory(${SDL3_DIR} ${CMAKE_CURRENT_BINARY_DIR}/SDL3)

# --- OpenAL Soft ---
message(STATUS "Configuring OpenAL Soft from submodule...")
set(OPENAL_DIR "${REVC_VENDOR_DIR}/openal-soft")
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
set(LIBTYPE "SHARED" CACHE STRING "" FORCE)
add_subdirectory(${OPENAL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/openal-soft)

# --- mpg123 (MP3 radio stations) ---
message(STATUS "Configuring mpg123 from submodule...")
set(MPG123_DIR "${REVC_VENDOR_DIR}/mpg123")
add_subdirectory(${MPG123_DIR} ${CMAKE_CURRENT_BINARY_DIR}/mpg123)

# --- GLFW no es necesario para Android - usamos SDL2 ---
# message(STATUS "Configuring GLFW from submodule...")
# set(GLFW_DIR "${REVC_VENDOR_DIR}/glfw")
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
# add_subdirectory(${GLFW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glfw)

# --- librw ---
message(STATUS "Configuring librw from submodule...")
set(LIBRW_DIR "${REVC_VENDOR_DIR}/librw")
set(LIBRW_PLATFORM "${REVC_RENDERER}" CACHE STRING "" FORCE)
set(LIBRW_GL3_GFXLIB "SDL3" CACHE STRING "" FORCE)
set(LIBRW_INSTALL OFF CACHE BOOL "" FORCE)
set(LIBRW_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIBRW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/librw)

# The Android debug variant is what we install while iterating, but the
# renderer does a lot of native geometry work. Keep symbols/debuggability while
# avoiding an effectively unoptimized renderer/game build.
target_compile_options(librw PRIVATE
    $<$<CONFIG:Debug>:-O3>
    $<$<CONFIG:Debug>:-ffast-math>
    $<$<CONFIG:Debug>:-ftree-vectorize>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

# Agregar includes de SDL3 a librw (necesario para gl3device.cpp)
target_include_directories(librw PUBLIC 
    ${SDL3_DIR}/include
)

# ============================================================================
# GitSHA1.cpp
# ============================================================================
set(GIT_SHA1_CONTENT "#define GIT_SHA1 \"android-build\"\nconst char* g_GIT_SHA1 = GIT_SHA1;\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/extras/GitSHA1.cpp" "${GIT_SHA1_CONTENT}")

# ============================================================================
# Fuentes de reVC
# ============================================================================
file(GLOB_RECURSE REVC_SOURCES 
    "${REVC_SRC_DIR}/*.cpp"
    "${REVC_SRC_DIR}/*.c"
)

# Excluir archivos de otras plataformas
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*sampman_miles\\.cpp$")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/win/.*")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/glfw/.*")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/demangle/.*")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*PoliceRadio\\.cpp$")

# Directorios de headers
file(GLOB_RECURSE ALL_HEADERS "${REVC_SRC_DIR}/*.h")
set(INCLUDE_DIRS "")
foreach(HEADER ${ALL_HEADERS})
    get_filename_component(DIR ${HEADER} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${DIR})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# ============================================================================
# Target principal
# ============================================================================
add_library(${EXECUTABLE} SHARED
    ${REVC_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/extras/GitSHA1.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/jni_bridge.cpp"
)

target_include_directories(${EXECUTABLE} PRIVATE
    ${REVC_SRC_DIR}
    ${INCLUDE_DIRS}
    ${REVC_SRC_DIR}/fakerw
    ${LIBRW_DIR}
    ${SDL3_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/SDL3/include
    ${CMAKE_CURRENT_BINARY_DIR}/SDL3/include/SDL3
    ${CMAKE_CURRENT_BINARY_DIR}/SDL3/include-config-$<LOWER_CASE:$<CONFIG>>/SDL3
    ${OPENAL_DIR}/include
    ${OPENAL_DIR}/include/AL
    ${MPG123_DIR}/src/include
)

target_compile_definitions(${EXECUTABLE} PRIVATE
    ANDROID=1
    _ANDROID=1
    LINUX=1
    _LINUX=1
    POSIX=1
    LIBRW
    LIBRW_SDL3
    AUDIO_OAL
    AUDIO_OAL_USE_MPG123
    RW_${REVC_RENDERER}
    CMAKE_BUILD
    USE_OUR_VERSIONING
    $<$<CONFIG:DEBUG>:DEBUG>
    $<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

target_compile_options(${EXECUTABLE} PRIVATE
    -w
    -fexceptions
    -frtti
    -fpermissive
    -funwind-tables
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-ffast-math>
    $<$<CONFIG:Release>:-fno-finite-math-only>
    $<$<CONFIG:Release>:-ftree-vectorize>
    $<$<CONFIG:RelWithDebInfo>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-ffast-math>
    $<$<CONFIG:RelWithDebInfo>:-fno-finite-math-only>
    $<$<CONFIG:RelWithDebInfo>:-ftree-vectorize>
    $<$<CONFIG:Debug>:-O3>
    $<$<CONFIG:Debug>:-ffast-math>
    $<$<CONFIG:Debug>:-ftree-vectorize>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

# Librerías del sistema Android
find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(${EXECUTABLE}
    SDL3::SDL3
    OpenAL
    libmpg123
    librw
    ${log-lib}
    ${android-lib}
)
if(REVC_RENDERER STREQUAL "VULKAN")
    find_library(vulkan-lib vulkan)
    target_link_libraries(${EXECUTABLE} ${vulkan-lib})
else()
    find_library(gles3-lib GLESv3)
    find_library(egl-lib EGL)
    target_link_libraries(${EXECUTABLE} ${gles3-lib} ${egl-lib})
endif()

message(STATUS "reVC Android build configured for ${ANDROID_ABI}")
