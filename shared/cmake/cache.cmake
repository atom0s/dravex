#
# Based on various cmake modules found here:
# https://github.com/aminya/project_options
#

include_guard()

# Enables compiler caching, if available.
function (enable_cache)
    # Set the cache type to use..
    set(CACHE_OPTION "ccache" STRING "The compiler cache to be used.")

    # Set the available options..
    set(CACHE_OPTION_VALUES "ccache" "sccache")
    set_property(CACHE CACHE_OPTION PROPERTY STRING ${CACHE_OPTION_VALUES})

    # Find the index of the selected cache option..
    list(FIND CACHE_OPTION_VALUES ${CACHE_OPTION} CACHE_OPTION_INDEX)

    # Ensure the desired option was found..
    if (${CACHE_OPTION_INDEX} EQUAL -1)
        message(STATUS "[INFO] Using custom compiler cache system: '${CACHE_OPTION}', supported: ${CACHE_OPTION_VALUES}")
    endif()

    # Find the desired cache option program..
    find_program(CACHE_BINARY NAMES ${CACHE_OPTION_VALUES})

    # Set the compiler caching options if found..
    if (CACHE_BINARY)
        message(STATUS "[INFO] Using '${CACHE_OPTION}' for compiler caching; path: ${CACHE_BINARY}")

        set(CMAKE_CXX_COMPILER_LAUNCHER ${CACHE_BINARY} CACHE FILEPATH "CXX compiler cache used")
        set(CMAKE_C_COMPILER_LAUNCHER ${CACHE_BINARY} CACHE FILEPATH "C compiler cache used")
    else()
        message(WARNING "[WARN] Caching was requested but the selected option was not found: ${CACHE_OPTION}")
    endif()
endfunction()