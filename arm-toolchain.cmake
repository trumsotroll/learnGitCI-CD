# Cross-compilation toolchain for ARM Cortex-M4 (S32K144)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Compilers
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size)

# CPU flags for Cortex-M4 with FPU
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS} --specs=nosys.specs --specs=nano.specs")

# Disable compiler test — cross-compiler cannot run on host
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)