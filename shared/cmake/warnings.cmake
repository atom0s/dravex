#
# Based on various cmake modules found here:
# https://github.com/aminya/project_options
#

include_guard()

function(set_project_warnings project_name)
    # Visual Studio Warning Settings
    set(MSVC_WARNINGS
        /W4             # Set the default warning level to 4..
        /permissive-    # Enforce standards conformance..

        #
        # Force Disabled Warnings
        #

        /wd4505         # unreferenced local function has been removed

        #
        # Force Enabled Warnings
        #

        /w14242         # conversion from 'type1' to 'type2', possible loss of data
        /w14640         # construction of local static object is not thread-safe
    )

    # Set the project warnings based on the current platform..
    if (MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    else()
        message(WARNING "[WARN] No compiler warnings to set for compiler: '${CMAKE_CXX_COMPILER_ID}'")
    endif()

    # Enable treating warnings as errors by default..
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" TRUE)

    if (WARNINGS_AS_ERRORS)
        if (MSVC)
            set(ERRORS "/WX")
        endif()
    endif()

    # Set the compiler options..
    target_compile_options(${project_name} INTERFACE ${ERRORS} ${PROJECT_WARNINGS})
endfunction()
