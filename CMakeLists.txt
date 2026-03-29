cmake_minimum_required(VERSION 3.20)

# ====== Firmware target (ARM cross-compile) ======
project(s32k144-bootloader C ASM)

set(CMAKE_C_STANDARD 11)

# CPU define
add_definitions(-DCPU_S32K144HFT0VLLT)

# Source files
file(GLOB SRC_FILES       src/*.c)
file(GLOB DRIVER_FILES    driver/src/*.c)
file(GLOB HAL_FILES       hal/src/*.c)
file(GLOB STARTUP_FILES   Project_Settings/Startup_Code/*.c)
file(GLOB STARTUP_C_FILES   Project_Settings/Startup_Code/*.c)
file(GLOB STARTUP_ASM_FILES Project_Settings/Startup_Code/*.S)

# Include paths
include_directories(
    src
    driver/inc
    hal/inc
    include
)

# Firmware executable
add_executable(firmware
    ${SRC_FILES}
    ${DRIVER_FILES}
    ${HAL_FILES}
    ${STARTUP_FILES}
)

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Project_Settings/Linker_Files/S32K144_64_flash.ld)
target_link_options(firmware PRIVATE -T${LINKER_SCRIPT})

set_target_properties(firmware PROPERTIES OUTPUT_NAME "firmware" SUFFIX ".elf")

# Generate .hex and .bin after build
add_custom_command(TARGET firmware POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware.elf firmware.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
)

# ====== Unit test target (host GCC) ======
add_executable(test_runner
    test/test.c
    test/Unity/unity.c
)

target_include_directories(test_runner PRIVATE
    test/Unity
    src
    include
)

target_compile_definitions(test_runner PRIVATE
    CPU_S32K144HFT0VLLT
)