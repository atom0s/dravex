#
# Based on various cmake modules found here:
# https://github.com/aminya/project_options
#

include_guard()

# Define the available options..
option(ENABLE_CLANG_TIDY "Execute clang-tidy on compile." OFF)
option(ENABLE_CLANG_TIDY_AUTO_FIX "Allow clang-tidy to automatically apply fixes to problems. (Dangerous!)" OFF)

# Display the options current values..
message(STATUS "         ENABLE_CLANG_TIDY: " ${ENABLE_CLANG_TIDY})
message(STATUS "ENABLE_CLANG_TIDY_AUTO_FIX: " ${ENABLE_CLANG_TIDY_AUTO_FIX})
message(STATUS "")

# Find and prepare clang-tidy if enabled..
if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)

    if (CLANG_TIDY_COMMAND)
        if (ENABLE_CLANG_TIDY_AUTO_FIX)
            set(CLANG_TIDY_AUTOFIX "-fix")
        endif()

        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND};-header-filter='${CMAKE_SOURCE_DIR}/src/*';${CLANG_TIDY_AUTOFIX};-format-style='file'")

        message(STATUS "[INFO] Using '${CLANG_TIDY_COMMAND}' for clang-tidy linting.")
    else()
        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)

        message(WARNING "[WARN] Linting was requested but could not find clang-tidy.")
    endif()
endif()