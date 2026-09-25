cmake_minimum_required(VERSION 3.22.1)

project("re3" LANGUAGES C CXX)

set(EXECUTABLE re3)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# On Windows, there is no libm (math lib is in standard C lib), but some Unix-centric
# scripts might try to link 'm'. We create a dummy interface library to satisfy 
# those requests without linking anything.
if(WIN32)
  add_library(m INTERFACE)
endif()

# ============================================================================
# Deshabilitar TODAS las warnings
# ============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

# Rutas del proyecto re3
set(REVC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(REVC_SRC_DIR "${REVC_ROOT}/src")
set(REVC_VENDOR_DIR "${REVC_ROOT}/vendor")

message(STATUS "REVC_ROOT: ${REVC_ROOT}")
message(STATUS "REVC_SRC_DIR: ${REVC_SRC_DIR}")

if(NOT EXISTS "${REVC_SRC_DIR}")
    message(FATAL_ERROR "Source directory not found: ${REVC_SRC_DIR}")
endif()

# ============================================================================
# Submodules - Usar dependencias locales en vendor/
# ============================================================================

# --- SDL3 ---
message(STATUS "Configuring SDL3 from submodule...")
set(SDL3_DIR "${REVC_VENDOR_DIR}/SDL3")
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(${SDL3_DIR} ${CMAKE_CURRENT_BINARY_DIR}/SDL3)

# --- OpenAL Soft ---
message(STATUS "Configuring OpenAL Soft from submodule...")
set(OPENAL_DIR "${REVC_VENDOR_DIR}/openal-soft")
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
# Don't try to link with math library on Windows - it's in standard lib
if(WIN32)
    # OpenAL uses HAVE_LIBM to decide if it should link against m.lib
    set(HAVE_LIBM OFF CACHE BOOL "" FORCE)
    set(MATH_LIB "" CACHE STRING "" FORCE)
    # Also clear MATH_LIBRARY just in case standard find_package uses it
    set(MATH_LIBRARY "" CACHE STRING "" FORCE)
endif()
add_subdirectory(${OPENAL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/openal-soft)

# --- mpg123 --- DISABLED FOR NOW, causing POSIX issues on Windows
# message(STATUS "Configuring mpg123 from submodule...")
# set(MPG123_DIR "${REVC_VENDOR_DIR}/mpg123")
# set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
# set(BUILD_LIBOUT123 OFF CACHE BOOL "" FORCE)
# add_subdirectory(${MPG123_DIR} ${CMAKE_CURRENT_BINARY_DIR}/mpg123)

# --- librw ---
message(STATUS "Configuring librw from submodule...")
set(LIBRW_DIR "${REVC_VENDOR_DIR}/librw")
set(LIBRW_PLATFORM "GL3" CACHE STRING "" FORCE)
set(LIBRW_GL3_GFXLIB "SDL3" CACHE STRING "" FORCE)
set(LIBRW_INSTALL OFF CACHE BOOL "" FORCE)
set(LIBRW_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIBRW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/librw)

# Agregar includes de SDL3 a librw
target_include_directories(librw PUBLIC 
    ${SDL3_DIR}/include
)

# Agregar flags de compilación permisivos a librw (igual que Android)
target_compile_options(librw PRIVATE -fpermissive)

# ============================================================================
# GitSHA1.cpp
# ============================================================================
set(GIT_SHA1_CONTENT "#define GIT_SHA1 \"windows-build\"\nconst char* g_GIT_SHA1 = GIT_SHA1;\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/extras/GitSHA1.cpp" "${GIT_SHA1_CONTENT}")

# ============================================================================
# Fuentes de re3
# ============================================================================
file(GLOB_RECURSE REVC_SOURCES 
    "${REVC_SRC_DIR}/*.cpp"
    "${REVC_SRC_DIR}/*.c"
    "${REVC_SRC_DIR}/*.rc"
)

# Excluir archivos de otras plataformas (NO Android/iOS, mantener win/)
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*sampman_miles\\.cpp$")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/android/.*")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/ios/.*")
list(FILTER REVC_SOURCES EXCLUDE REGEX ".*/osx/.*")
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
# Target principal - EXECUTABLE para Windows, no shared library
# ============================================================================
add_executable(${EXECUTABLE}
    ${REVC_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/extras/GitSHA1.cpp"
)

target_include_directories(${EXECUTABLE} PRIVATE
    ${REVC_SRC_DIR}
    ${INCLUDE_DIRS}
    ${REVC_SRC_DIR}/fakerw
    ${LIBRW_DIR}
    ${SDL3_DIR}/include
    ${OPENAL_DIR}/include
    ${OPENAL_DIR}/include/AL
    # ${MPG123_DIR}/src/include  # Disabled - not using mpg123
    # ${MPG123_DIR}/src/libmpg123 # Disabled - not using mpg123
)

target_compile_definitions(${EXECUTABLE} PRIVATE
    LIBRW
    LIBRW_SDL3
    # AUDIO_OAL  # Disabled - Using NULL audio as per user request to avoid crashes
    # AUDIO_OAL_USE_MPG123  # Disabled - mpg123 has POSIX issues on Windows
    # RW_GL3  # Already defined below? No, it's defined here. Keep it?
    # AUDIO_OAL is what triggers the OAL backend.
    RW_GL3
    CMAKE_BUILD
    USE_OUR_VERSIONING
    LIBRW_PLATFORM_GL3
    NDEBUG  # Always NDEBUG to avoid heap corruption checks
)

target_compile_options(${EXECUTABLE} PRIVATE
    -w
    -fexceptions
    -frtti
    -fpermissive
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-ffast-math>
)

# Librerías para Windows
target_link_libraries(${EXECUTABLE}
    SDL3::SDL3
    # OpenAL  # Disabled - Using NULL audio
    # libmpg123  # Disabled - POSIX issues on Windows
    librw
    # Windows system libraries
    opengl32
    ws2_32
    winmm
    imm32
    version
    setupapi
    gdi32
    user32
    ole32
    oleaut32
    shell32
    advapi32
    uuid
    # Note: Math library (m) not needed on Windows - functions are in standard lib
)

message(STATUS "re3 Windows build configured")
