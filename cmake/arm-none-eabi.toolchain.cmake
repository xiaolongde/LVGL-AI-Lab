# arm-none-eabi.toolchain.cmake — GCC ARM 交叉编译 toolchain for GD32F303RCT6
#
# Cortex-M4 + FPU (single precision)，hard-float ABI。
#
# Usage:
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.toolchain.cmake

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  arm)

# 跨平台编译时 try_compile 会想跑 link，但裸机环境无 startup → 改成只产 STATIC_LIBRARY
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 工具链可执行文件（依赖 PATH，已确认 arm-none-eabi-gcc 12.2 在）
set(CMAKE_C_COMPILER     arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER   arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER   arm-none-eabi-gcc)
set(CMAKE_OBJCOPY        arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP        arm-none-eabi-objdump)
set(CMAKE_SIZE           arm-none-eabi-size)

# CPU + FPU flags
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# 公共编译选项（C / C++ / ASM 共享）
set(COMMON_FLAGS "${MCU_FLAGS} -ffunction-sections -fdata-sections -fno-common -Wall -Wextra")

set(CMAKE_C_FLAGS_INIT     "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS_INIT   "${COMMON_FLAGS} -std=gnu++17 -fno-rtti -fno-exceptions")
set(CMAKE_ASM_FLAGS_INIT   "${MCU_FLAGS} -x assembler-with-cpp")

# 链接选项：gc-sections 删未引用 / 用 newlib-nano 减小 size / 不要 system libs 重定义
set(CMAKE_EXE_LINKER_FLAGS_INIT "\
${MCU_FLAGS} \
-Wl,--gc-sections \
-Wl,--print-memory-usage \
--specs=nano.specs --specs=nosys.specs \
")

# Debug / Release 默认
set(CMAKE_C_FLAGS_DEBUG_INIT     "-Og -g3")
set(CMAKE_CXX_FLAGS_DEBUG_INIT   "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE_INIT   "-Os -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Os -g")
