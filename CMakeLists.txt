# Require newer CMAKE version..
cmake_minimum_required(VERSION 3.22)

# Define the toolchain to use vcpkg..
set(CMAKE_TOOLCHAIN_FILE "Z:/SourceCode/deps/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(Z_VCPKG_BUILTIN_POWERSHELL_PATH "C:/Program Files/Powershell/7/pwsh.exe")

# Define the project..
project(dravex CXX)

# Setup properties..
set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY CXX_EXTENSIONS ON)
set_property(GLOBAL PROPERTY LINKER_LANGUAGE CXX)

# Setup the CMAKE modules path..
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shared/cmake)

# Include the local CMAKE modules..
include(cache)
include(platform)
include(settings)
include(warnings)

# Enable position independent libraries..
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use pthreads if available..
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

# Link the 'library' to the requested C++ options/standard..
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

# Link the warnings defined in the warnings.cmake..
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

# Display build information..
message(STATUS "             CMAKE_VERSION: ${CMAKE_VERSION}")
message(STATUS "          CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "          CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "        CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "     CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "           CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "           CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "  CMAKE_GENERATOR_PLATFORM: ${CMAKE_GENERATOR_PLATFORM}")

# Setup parallel building if not already set by the user..
if (NOT CMAKE_BUILD_PARALLEL_LEVEL)
    # Obtain the local processor count..
    include(ProcessorCount)
    ProcessorCount(N)

    # Set the count if multiple cores are available..
    if (NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    endif()
endif()
message(STATUS "CMAKE_BUILD_PARALLEL_LEVEL: ${CMAKE_BUILD_PARALLEL_LEVEL}")

# Include ClangTidy..
include(clangtidy)

# Include zlib..
find_package(ZLIB REQUIRED)

#
# Application Settings
#

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(dravex_inc
        "ext/d3d9/include/"
        "ext/imgui/"
        "ext/imgui_fontawesome/"
        "ext/zlib/"
    )
    set(dravex_lib_paths
        "ext/d3d9/lib/x64/"
    )
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(dravex_inc
        "ext/d3d9/include/"
        "ext/imgui/"
        "ext/imgui_fontawesome/"
        "ext/zlib/"
    )
    set(dravex_lib_paths
        "ext/d3d9/lib/x86/"
    )
endif()

set(dravex_lib
    "d3dx9"
    "dxerr"
    ZLIB::ZLIB
)
set(dravex_src
    "src/binarybuffer.hpp"
    "src/defines.hpp"
    "src/imgui_dravex.cpp"
    "src/imgui_dravex.hpp"
    "src/main.cpp"
    "src/utils.hpp"
    "src/window.cpp"
    "src/window.hpp"

    "src/assets/asset.hpp"
    "src/assets/asset_texture.hpp"

    "src/package/package.cpp"
    "src/package/package.hpp"
    "src/package/v118.hpp"
    "src/package/v666.hpp"

    # ImGui Source Files
    "ext/imgui/imgui_demo.cpp"
    "ext/imgui/imgui_draw.cpp"
    "ext/imgui/imgui_tables.cpp"
    "ext/imgui/imgui_widgets.cpp"
    "ext/imgui/imgui.cpp"
)

if (WIN32)
    set(dravex_res "${CMAKE_SOURCE_DIR}/res/dravex.rc")
endif()

add_executable(dravex ${dravex_src} ${dravex_res})
target_include_directories(dravex PUBLIC ${dravex_inc})
target_link_directories(dravex PUBLIC ${dravex_lib_paths})
target_link_libraries(dravex ${dravex_lib})

if (WIN32)
    set_target_properties(dravex PROPERTIES
        OUTPUT_NAME dravex
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()
