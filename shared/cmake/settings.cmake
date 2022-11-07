#
# Based on various cmake modules found here:
# https://github.com/aminya/project_options
#

include_guard()

# Prevent in-source builds..
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

if ("${srcdir}" STREQUAL "${bindir}")
    message(SEND_ERROR "[ERR] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    message(SEND_ERROR "[ERR] In-source builds are not allowed.")
    message(SEND_ERROR "[ERR] Please make a separate build directory and run cmake from there.")
    message(SEND_ERROR "[ERR] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    message(FATAL_ERROR "Critical error; cannot continue.")
endif()

# Set the default build type if none was selected..
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "[INFO] Defaulting to build type 'RelWithDebInfo' as none was specified..")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Enable generating compile_commands.json..
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup enhanced compiler messages and error reporting..
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER 1900)
    add_compile_options(/diagnostics:column)
else()
    message(STATUS "[INFO] No enhanced compiler diagnostics set for compiler: '${CMAKE_CXX_COMPILER_ID}'")
endif()

# Ensure the C++ standards is set..
if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    if (DEFINED CMAKE_CXX23_STANDARD_COMPILE_OPTION OR DEFINED CMAKE_CXX23_EXTENSION_COMPILE_OPTION)
        set(CXX_LATEST_STANDARD 23)
    elseif (DEFINED CMAKE_CXX20_STANDARD_COMPILE_OPTION OR DEFINED CMAKE_CXX20_EXTENSION_COMPILE_OPTION)
        set(CXX_LATEST_STANDARD 20)
    elseif (DEFINED CMAKE_CXX17_STANDARD_COMPILE_OPTION OR DEFINED CMAKE_CXX17_EXTENSION_COMPILE_OPTION)
        set(CXX_LATEST_STANDARD 17)
    elseif (DEFINED CMAKE_CXX14_STANDARD_COMPILE_OPTION OR DEFINED CMAKE_CXX14_EXTENSION_COMPILE_OPTION)
        set(CXX_LATEST_STANDARD 14)
    else()
        set(CXX_LATEST_STANDARD 11)
    endif()
    set(CMAKE_CXX_STANDARD ${CXX_LATEST_STANDARD})
endif()

# Set option for link-time code generation..
option(ENABLE_IPO "Enable IPO/LTO (interprocedural optimizations)" ON)

if (ENABLE_IPO)
    # Check if IPO is supported..
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)

    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "[INFO] IPO/LTO was enabled but is not supported: ${output}")
    endif()
endif()

# Set option for supporting fast math optimizations..
option(ENABLE_FAST_MATH "Enable fast math optimizations" ON)

if (ENABLE_FAST_MATH)
    if ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
        add_compile_options(-ffast-math)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/fp:fast)
    endif()
else()
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/fp:precise)
    endif()
endif()

# Remove default settings..
if (MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Adjust MSVC specific settings..
if (MSVC)
    list(APPEND FLAGS_AND_DEFINES
        -D_CONSOLE      # Define _CONSOLE application type..
        -D_MBCS         # Define _MBCS (multibyte character set)..
        /MP             # Enable multi-processor compiling..
        /GS             # Enable securiy checks..
        /analyze-       # Disable code analysis..
        /Zc:wchar_t     # Treat wchar_t as built-in type..
        /Zc:inline      # Remove unreferenced code..
        /Zc:forScope    # Force comformance in for loop scope..
        /Zi             # Enable program database..
        /Gm-            # Disable minimal rebuild..
        /Gy-            # Disable function-level linking..
        /sdl            # Enable SDL checks..
        /GF             # Enable string pooling..
        /Gd             # Default calling convention (__cdecl)
        /Oy-            # Don't omit frame pointers..
        /FC             # Use full paths in diagnostic messages..
        /EHa            # Use C++ exceptions with SEH..
        /bigobj         # Allow for bigger object files..
        )

    # Add configuration type specific settings..
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Compiler settings..
        list(APPEND FLAGS_AND_DEFINES
            -D_DEBUG    # Define _DEBUG
            /Od         # Disable optimizations..
            /RTC1       # Enable basic runtime checks..
            /MDd        # Use Multi-threaded Debug DLL Runtime..
            )

        # Linker settings..
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NXCOMPAT /DYNAMICBASE /DEBUG /INCREMENTAL /SUBSYSTEM:CONSOLE")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO /NXCOMPAT /DYNAMICBASE /DEBUG /INCREMENTAL /DLL /SUBSYSTEM:WINDOWS")
    else()
        # Compiler settings..
        list(APPEND FLAGS_AND_DEFINES
            -D_NDEBUG   # Define _NDEBUG
            /GL         # Enable whole program optimization..
            /Oi         # Enable intrinsic functions..
            /O2         # Enable maximum optmization (favor speed)..
            /MD         # Use Multi-threaded DLL Runtime..
            )

        # Linker settings..
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /LTCG:incremental /NXCOMPAT /DYNAMICBASE /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /OPT:ICF /DEBUG:NONE /NOCOFFGRPINFO")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO /LTCG:incremental /NXCOMPAT /DYNAMICBASE /OPT:REF /INCREMENTAL:NO /DLL /SUBSYSTEM:WINDOWS /OPT:ICF /DEBUG:NONE /NOCOFFGRPINFO")
    endif()

    # Set overall linker flags..
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libci")
endif()

# Set the CMAKE C++ flags..
string(REPLACE ";" " " FLAGS_AND_DEFINES_STR "${FLAGS_AND_DEFINES}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_AND_DEFINES_STR}")

# Sets the target output directory for the additional compiler generated files.
function (set_target_output_directory target)
    message(STATUS "[INFO] Setting output directory for ${target} to ${CMAKE_SOURCE_DIR}")

    set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
        )
endfunction()