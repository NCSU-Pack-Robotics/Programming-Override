cmake_minimum_required(VERSION 3.22.1)

# Set C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)   # this makes it "gnu2x" instead of "c23"
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON) # this makes it "gnu++23"

#set(CMAKE_TOOLCHAIN_FILE toolchain-arm-none-eabi.cmake CACHE STRING "Toolchain file")

project(Programming_Push_Back C CXX ASM)

##################################################################################
# Definitions and Options
##################################################################################

# Options to include/exclude parts of the codebase
option(THINK "Include THINK robot's code" ON)
option(DO "Include DO robot's code" OFF)
option(BRAIN "Code will run on the VEX Brain" ON)
option(PI "Code will run on a Raspberry Pi" OFF)
option(TESTING "Include test code" OFF)

# Create interface library for definitions from the options
add_library(config INTERFACE)
target_compile_definitions(config INTERFACE
        THINK=$<BOOL:${THINK}>
        DO=$<BOOL:${DO}>
        BRAIN=$<BOOL:${BRAIN}>
        PI=$<BOOL:${PI}>
)

# Check for liblvgl llemu files and define macros if they exist
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/include/liblvgl/llemu.h")
    add_definitions(-D_PROS_INCLUDE_LIBLVGL_LLEMU_H)
endif ()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/include/liblvgl/llemu.hpp")
    add_definitions(-D_PROS_INCLUDE_LIBLVGL_LLEMU_HPP)
endif ()

##################################################################################
# Find necessary files, libraries, and subdirectories
##################################################################################

# Files  TODO: This glob method of finding source files is actively discouraged by CMake
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        Push_Back/src/*.cpp
        Push_Back/src/*.hpp
        Push_Back/src/*.c
        Push_Back/src/*.h)
list(FILTER SRC_FILES EXCLUDE REGEX ".*/test/.*")
file(GLOB TOP_LEVEL_INCLUDE CONFIGURE_DEPENDS
        Push_Back/include/*.h)
file(GLOB_RECURSE PROS_FILES CONFIGURE_DEPENDS
        Push_Back/include/pros/*.cpp
        Push_Back/include/pros/*.hpp
        Push_Back/include/pros/*.c
        Push_Back/include/pros/*.h)
file(GLOB_RECURSE COMMON_FILES CONFIGURE_DEPENDS
        Push_Back/dependencies/common/*.cpp
        Push_Back/dependencies/common/*.hpp
        Push_Back/dependencies/common/*.c
        Push_Back/dependencies/common/*.h)
file(GLOB FIRMWARE_LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/firmware/*.a
)

# Exclude build and external directories from source/header globs
list(FILTER SRC_FILES EXCLUDE REGEX ".*/cmake-build-.*|.*/_deps/.*|.*/CMakeFiles/.*")

# Directories to include in compilation
include_directories(
        Push_Back/src
        Push_Back/include
        Push_Back/include/pros
)

##################################################################################
# Build dependency libraries
##################################################################################

list(APPEND BUILT_LIBS)
add_subdirectory(Push_Back/dependencies)

##################################################################################
# Everything Testing
##################################################################################
if (TESTING)  # Should be after dependencies so they can be used in test build
    add_subdirectory("Push_Back/test")
endif ()

##################################################################################
# Compilation & Linking flags
##################################################################################

if (NOT TESTING)

# Linker flags and scripts
set(LINKER_SCRIPT_HOT ${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/firmware/v5-hot.ld)
set(LINKER_SCRIPT_COLD ${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/firmware/v5.ld)
set(LINKER_SCRIPT_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/Push_Back/firmware/v5-common.ld)

# All the flags to use later
set(MFLAGS
        -mcpu=cortex-a9
        -mfpu=neon-fp16
        -mfloat-abi=hard
        -Os
        -g
        -mthumb
)
set(CPP_FLAGS
        -D_POSIX_THREADS
        -D_UNIX98_THREAD_MUTEX_ATTRIBUTES
        -D_POSIX_TIMERS
        -D_POSIX_MONOTONIC_CLOCK
)
set(GCCFLAGS
        -ffunction-sections
        -fdata-sections
        -fdiagnostics-color
        -funwind-tables
)
set(WARNFLAGS
        -Wno-psabi
        -Wno-deprecated-declarations  # PROS uses lots of deprecated functions internally
)

# Combine flags
add_compile_options(${CPP_FLAGS} ${MFLAGS} ${GCCFLAGS} ${WARNFLAGS})

# Add extra flags from environment or command line (CMake treats undefined variables as empty)
string(APPEND CMAKE_C_FLAGS " $ENV{EXTRA_CFLAGS}")
string(APPEND CMAKE_CXX_FLAGS " $ENV{EXTRA_CXXFLAGS}")

endif ()

##################################################################################
# Create timestamp
##################################################################################

if (NOT TESTING)

# File paths
set(TIMESTAMP_SRC ${CMAKE_BINARY_DIR}/_pros_ld_timestamp.c)
set(TIMESTAMP_TEMPLATE ${TIMESTAMP_SRC}.in)

# Create template file
file(WRITE ${TIMESTAMP_TEMPLATE}
"const int _PROS_COMPILE_TIMESTAMP_INT = @TIMESTAMP_INT@;
const char * const _PROS_COMPILE_TIMESTAMP = \"@TIMESTAMP_STR@\";
const char * const _PROS_COMPILE_DIRECTORY = \"@COMPILE_DIR@\";"
)

# Get Unix timestamp
execute_process(
        COMMAND date +%s
        OUTPUT_VARIABLE UNIX_TIME
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get timezone offset in hours (e.g., -0500 → -5)
execute_process(
        COMMAND date +%-z
        OUTPUT_VARIABLE TZ_OFFSET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Adjust timestamp by timezone offset and convert to int
math(EXPR TIMESTAMP_INT "${UNIX_TIME} + (${TZ_OFFSET} / 100 * 3600)")

# Create formatted timestamp like __DATE__ " " __TIME__
string(TIMESTAMP TIMESTAMP_STR "%b %d %Y %H:%M:%S")

# Get last 23 characters of working directory
get_filename_component(PWD "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
string(LENGTH "${PWD}" PWD_LEN)

# Truncate to last 23 characters if too long
if (PWD_LEN GREATER 23)
    math(EXPR START "${PWD_LEN} - 23")
    string(SUBSTRING "${PWD}" ${START} 23 COMPILE_DIR)
else ()
    set(COMPILE_DIR "${PWD}")
endif ()

# Generate the timestamp source file from the template
configure_file(
        ${TIMESTAMP_SRC}.in
        ${TIMESTAMP_SRC}
        @ONLY
)

endif ()

##################################################################################
# Find project.pros file for moving bin files to expected location.
##################################################################################

if (NOT TESTING)

# Find the directory containing pros.project file
file(GLOB_RECURSE PROS_PROJECT_FILE "${CMAKE_SOURCE_DIR}/*/project.pros")
if (PROS_PROJECT_FILE)
    list(GET PROS_PROJECT_FILE 0 PROS_PROJECT_PATH)
    get_filename_component(PROS_PROJECT_DIR "${PROS_PROJECT_PATH}" DIRECTORY)
    message(STATUS "Found pros.project at: ${PROS_PROJECT_DIR}")
    file(MAKE_DIRECTORY ${PROS_PROJECT_DIR}/bin)
else ()
    message(WARNING "Could not find project.pros file")
    set(PROS_PROJECT_DIR "${CMAKE_SOURCE_DIR}")
endif ()

endif ()

##################################################################################
# Build Cold Package
##################################################################################

if (NOT TESTING)

# Libraries to link
set(COLD_FIRMWARE_LIBS ${FIRMWARE_LIBS})
# Cannot include newlib and libc because not all of the req'd stubs are implemented
list(FILTER COLD_FIRMWARE_LIBS EXCLUDE REGEX "libc\.a$")
list(FILTER COLD_FIRMWARE_LIBS EXCLUDE REGEX "libm\.a$")

# Create generator expressions for built libraries
set(BUILT_LIBS_GENEX "")
foreach(lib IN LISTS BUILT_LIBS)
    list(APPEND BUILT_LIBS_GENEX "$<TARGET_FILE:${lib}>")
    message(STATUS "Added generator expression for: ${lib}")
endforeach()

#target_link_libraries(${COLD_FIRMWARE_LIBS} config)
list(TRANSFORM COLD_FIRMWARE_LIBS PREPEND "-Wl," OUTPUT_VARIABLE COLD_FIRMWARE_LIBS_WL)
list(TRANSFORM FIRMWARE_LIBS PREPEND "-Wl," OUTPUT_VARIABLE FIRMWARE_LIBS_WL)

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/cold.package.elf
        COMMAND ${CMAKE_LINKER}
            ${MFLAGS}
            ${WARNFLAGS}
            -nostdlib
            ${GCCFLAGS}
            -Wl,--gc-keep-exported
            -Wl,--whole-archive
            ${COLD_FIRMWARE_LIBS_WL}
            -Wl,-lstdc++
            -Wl,--no-whole-archive
            -T${LINKER_SCRIPT_COLD}
            -Wl,--gc-sections
            -Wl,--start-group
            ${FIRMWARE_LIBS_WL}
            -Wl,-lgcc
            -Wl,-lc
            -Wl,-lm
            -Wl,-lnosys
            -Wl,--end-group
            -T${LINKER_SCRIPT_COMMON}
            -Wl,--no-warn-rwx-segments
            -Wl,--sort-section=alignment
            -Wl,--sort-common
            ${BUILT_LIBS_GENEX}
            -o ${CMAKE_BINARY_DIR}/cold.package.elf
        DEPENDS ${COLD_FIRMWARE_LIBS} ${BUILT_LIBS}
        COMMENT "Linking cold.package.elf"
)

# Create binary
add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/cold.package.bin
        DEPENDS ${CMAKE_BINARY_DIR}/cold.package.elf
        COMMAND ${CMAKE_OBJCOPY}
            --strip-symbol=install_hot_table
            --strip-symbol=__libc_init_array
            --strip-symbol=_PROS_COMPILE_DIRECTORY
            --strip-symbol=_PROS_COMPILE_TIMESTAMP
            --strip-symbol=_PROS_COMPILE_TIMESTAMP_INT
            ${CMAKE_BINARY_DIR}/cold.package.elf
        COMMAND ${CMAKE_OBJCOPY}
            -O binary
            -R .hot_init
            ${CMAKE_BINARY_DIR}/cold.package.elf
            ${CMAKE_BINARY_DIR}/cold.package.bin
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/cold.package.bin
            ${PROS_PROJECT_DIR}/bin/cold.package.bin
        COMMENT "Creating cold.package.bin"
)

add_custom_target(cold_package DEPENDS ${CMAKE_BINARY_DIR}/cold.package.bin)

endif ()

##################################################################################
# Build Hot Package
##################################################################################

if (NOT TESTING)

# Embed every file in Push_Back/static/ into the firmware.
set(ASSET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Push_Back")
set(ASSET_SOURCE_DIR "${ASSET_ROOT}/static")

# Find all assets recursively.
file(GLOB_RECURSE ASSET_FILES
        CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${ASSET_SOURCE_DIR}/*"
)

set(ASSET_OBJECTS "")

foreach(ASSET_FILE IN LISTS ASSET_FILES)

    # Get the path relative to Push_Back.
    # Example: static/test.txt
    file(RELATIVE_PATH ASSET_RELATIVE
            "${ASSET_ROOT}"
            "${ASSET_FILE}"
    )

    # Preserve the relative path to avoid object-file name collisions.
    # Example: build/assets/static/test.txt.o
    set(ASSET_OBJECT
            "${CMAKE_CURRENT_BINARY_DIR}/assets/${ASSET_RELATIVE}.o"
    )

    get_filename_component(ASSET_OBJECT_DIR
            "${ASSET_OBJECT}"
            DIRECTORY
    )

    add_custom_command(
            OUTPUT "${ASSET_OBJECT}"

            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${ASSET_OBJECT_DIR}"

            COMMAND "${CMAKE_OBJCOPY}"
            -I binary
            -O elf32-littlearm
            -B arm
            --rename-section
            .data=.rodata,alloc,load,readonly,data,contents
            "${ASSET_RELATIVE}"
            "${ASSET_OBJECT}"

            WORKING_DIRECTORY "${ASSET_ROOT}"

            DEPENDS "${ASSET_FILE}"

            COMMENT "Embedding ${ASSET_RELATIVE}"

            VERBATIM
    )

    list(APPEND ASSET_OBJECTS "${ASSET_OBJECT}")

endforeach()

message(STATUS "Assets to embed: ${ASSET_FILES}")

# Hot Package
add_executable(hot.package.elf
        ${SRC_FILES}
        ${TOP_LEVEL_INCLUDE}
        ${TIMESTAMP_SRC}
        ${ASSET_OBJECTS}
)

add_dependencies(hot.package.elf
        cold_package
        ${BUILT_LIBS})

target_link_libraries(hot.package.elf PRIVATE
        -Wl,--start-group
        ${FIRMWARE_LIBS}
        ${BUILT_LIBS}
        gcc
        stdc++
        c
        m
        nosys
        -Wl,--end-group
        config
)

target_link_options(hot.package.elf PRIVATE
        -T${LINKER_SCRIPT_HOT}
        ${MFLAGS}
        ${WARNFLAGS}
        ${GCCFLAGS}
        -T${LINKER_SCRIPT_COMMON}
        -Wl,--gc-sections
        -Wl,--no-warn-rwx-segments
        -Wl,--sort-section=alignment
        -Wl,--sort-common
        -nostdlib
        -nostartfiles
        -Wl,-R,${CMAKE_BINARY_DIR}/cold.package.elf
)

# Create binary
add_custom_command(
        TARGET hot.package.elf
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            ${CMAKE_BINARY_DIR}/hot.package.elf
            -O binary ${CMAKE_BINARY_DIR}/hot.package.bin
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/hot.package.bin
            ${PROS_PROJECT_DIR}/bin/hot.package.bin
        COMMENT "Creating hot.package.bin"
)

endif ()

##################################################################################
# Main Targets
##################################################################################

if (NOT TESTING)

set(FINAL_OUTPUTS
        cold_package
        hot.package.elf
)

# Hot and Cold package target
add_custom_target(Build
        DEPENDS ${FINAL_OUTPUTS}
)

add_custom_target(Build_and_Upload
        COMMAND cd ${PROS_PROJECT_DIR} && pros upload
        DEPENDS Build
        COMMENT "Uploading to V5 Brain"
)

add_custom_target(Run
        COMMAND cd ${PROJECT_SOURCE_DIR} && pros v5 run
        COMMENT "Running on V5 Brain"
)

add_custom_target(Stop
        COMMAND cd ${PROJECT_SOURCE_DIR} && pros v5 stop
        COMMENT "Stopping program on V5 Brain"
)

add_custom_target(Terminal
        COMMAND cd ${PROJECT_SOURCE_DIR} && pros terminal
        COMMENT "Opening terminal for V5 Brain"
)
endif ()
