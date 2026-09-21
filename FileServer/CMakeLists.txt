cmake_minimum_required(VERSION 3.20)

project(FileServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Dependencies
# ============================================================

include(FetchContent)

FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG master
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(httplib)

# ============================================================
# Source files
# ============================================================

set(SOURCES
        src/main.cpp

        src/Config.cpp
        src/Config.h

        src/FileServer.cpp
        src/FileServer.h

        src/PathUtils.cpp
        src/PathUtils.h
)

# ============================================================
# Executable
# ============================================================

add_executable(FileServer
        ${SOURCES}
)

# ============================================================
# Include directories
# ============================================================

target_include_directories(FileServer
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================
# Libraries
# ============================================================

target_link_libraries(FileServer
        PRIVATE
        httplib::httplib
)

# ============================================================
# Windows
# ============================================================

if (WIN32)
    target_compile_definitions(FileServer
            PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
endif()