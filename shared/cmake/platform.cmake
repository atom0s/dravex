# Check if the source path has any spaces..
if ("${CMAKE_SOURCE_DIR}" MATCHES " +")
    # Replace the paths spaces with underscores..
    set(STRIPPED_PATH "")
    STRING(REGEX REPLACE " +" "_" STRIPPED_PATH "${CMAKE_SOURCE_PATH}")

    # Suggest a cleaner path to the user..
    message(WARNING
        "[WARN] Your source directory path contains spaces; this is not recommended.\n"
        "[WARN] Current Path: ${CMAKE_SOURCE_DIR}\n"
        "[WARN] Suggested Path: ${CMAKE_SOURCE_DIR}")
endif()

# Disable system environment path for find calls..
if (WIN32)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
endif()