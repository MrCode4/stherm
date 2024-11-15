# Define URLs for downloading protobufLibs
# set(UNIX_PROTOBUF_URL "http://example.com/unix.tar.gz")
# set(WIN_PROTOBUF_URL "http://example.com/win.zip")

# Define the destination directory for extraction
set(UNIX_PROTOBUF_DIR "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/unix")
set(WIN_PROTOBUF_DIR "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/win")

if(UNIX)
    # Check if '3rdparty/protobufLibs/unix' directory exists
    if(NOT EXISTS "${UNIX_PROTOBUF_DIR}")
        message(STATUS "Directory 3rdparty/protobufLibs/unix not found.")

        # Check if 'unix.tar.gz' file exists
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/unix.tar.gz")
            message(FATAL_ERROR "File 3rdparty/protobufLibs/unix.tar.gz does not exist! Please download it manually.")

            # Download the file
            # message(STATUS "File 3rdparty/protobufLibs/unix.tar.gz not found. Downloading...")
            # file(DOWNLOAD ${UNIX_PROTOBUF_URL} "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/unix.tar.gz" SHOW_PROGRESS)
        endif()

        # Extract the downloaded tar.gz file
        message(STATUS "Extracting 3rdparty/protobufLibs/unix.tar.gz...")
        execute_process(
            COMMAND tar -xzvf "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/unix.tar.gz"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs"
            RESULT_VARIABLE result
            ERROR_VARIABLE error_msg
        )

        if(result)
            message(FATAL_ERROR "Failed to extract unix.tar.gz: ${error_msg}")
        else()
            message(STATUS "Extraction completed.")
        endif()

    endif()
elseif(WIN32)
    # Check if '3rdparty/protobufLibs/win' directory exists
    if(NOT EXISTS "${WIN_PROTOBUF_DIR}")
        message(STATUS "Directory 3rdparty/protobufLibs/win not found.")

        # Check if 'win.zip' file exists
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/win.zip")
            message(FATAL_ERROR "File 3rdparty/protobufLibs/win.zip does not exist! Please download it manually.")

            # Download the file
            # message(STATUS "File 3rdparty/protobufLibs/win.zip not found. Downloading...")
            # file(DOWNLOAD ${WIN_PROTOBUF_URL} "${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/win.zip" SHOW_PROGRESS)
        endif()

        # Extract the downloaded zip file
        message(STATUS "Extracting 3rdparty/protobufLibs/win.zip...")
        execute_process(
            COMMAND powershell -Command "Expand-Archive -Path '${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/win.zip' -DestinationPath '${CMAKE_SOURCE_DIR}/3rdparty/protobufLibs/win'"
            RESULT_VARIABLE result
            ERROR_VARIABLE error_msg
        )

        if(result)
            message(FATAL_ERROR "Failed to extract win.zip: ${error_msg}")
        else()
            message(STATUS "Extraction completed.")
        endif()

    endif()
else()
    message(STATUS "Unsupported platform. Only Linux and Windows are supported in this script.")
endif()
