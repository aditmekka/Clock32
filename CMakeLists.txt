cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Clock32)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# U8g2 display library sources
file(GLOB U8G2_SOURCES
    Drivers/u8g2/csrc/*.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/hmi.c
    Core/Src/display_draw.c
    Core/Src/u8g2_stm32_hal.c
    Core/Src/get_time.c
    Core/Src/uart1.c
    Core/Src/app.c
    ${U8G2_SOURCES}
    # TODO: re-add the sources below as the modules are implemented
    # Core/Src/ssd1306.c
    # Core/Src/battery.c
    # Core/Src/buzzer.c
    # Core/Src/ds3231.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/u8g2/csrc
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Generate flashable firmware images (.hex / .bin) from the .elf,
# alongside the linker map. Produced next to the .elf in the build dir.
# Derive the toolchain bin directory (it is not guaranteed to be on PATH at
# build time), so objcopy/size are always resolved to their full paths.
get_filename_component(TOOLCHAIN_BIN_DIR "${CMAKE_C_COMPILER_AR}" DIRECTORY)
if(WIN32)
    set(TOOLCHAIN_OBJCOPY "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-objcopy.exe")
else()
    set(TOOLCHAIN_OBJCOPY "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-objcopy")
endif()

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${TOOLCHAIN_OBJCOPY} -O ihex   $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${TOOLCHAIN_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating ${CMAKE_PROJECT_NAME}.hex / .bin firmware images"
)
