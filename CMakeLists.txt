# LVGL-AI-Lab — top-level CMake
#
# 平台: GD32F303RCT6 (Cortex-M4F, 256K Flash, 48K SRAM, 120 MHz)
# 工具链: arm-none-eabi-gcc 12.2 (走 cmake/arm-none-eabi.toolchain.cmake)

cmake_minimum_required(VERSION 3.20)

project(lvgl-ai-lab
    LANGUAGES C ASM
    VERSION 0.1.0
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 没指定就走 Debug（开发友好）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ---------------------------------------------------------------------------
# 路径常量
# ---------------------------------------------------------------------------
set(VENDOR_DIR  ${CMAKE_SOURCE_DIR}/vendor/gd32f30x)
set(SRC_DIR     ${CMAKE_SOURCE_DIR}/src)
set(LINKER_FILE ${CMAKE_SOURCE_DIR}/linker/gd32f303rc.ld)
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/startup/startup_gd32f303rc.S)

# ---------------------------------------------------------------------------
# Vendor: GD32F30x firmware library (CMSIS + standard_peripheral)
# ---------------------------------------------------------------------------
file(GLOB GD32_PERIPH_SRC CONFIGURE_DEPENDS
    "${VENDOR_DIR}/GD32F30x_standard_peripheral/Source/*.c"
)
set(GD32_SYSTEM_SRC
    "${VENDOR_DIR}/CMSIS/GD/GD32F30x/Source/system_gd32f30x.c"
)

add_library(gd32f30x STATIC
    ${GD32_PERIPH_SRC}
    ${GD32_SYSTEM_SRC}
)

target_include_directories(gd32f30x PUBLIC
    ${VENDOR_DIR}/CMSIS
    ${VENDOR_DIR}/CMSIS/GD/GD32F30x/Include
    ${VENDOR_DIR}/GD32F30x_standard_peripheral/Include
    ${SRC_DIR}                              # gd32f30x_libopt.h 在这（被 lib 间接 include）
)

# SoC family + clock 关键宏；PUBLIC 以让 link 该 lib 的 target 也拿到
target_compile_definitions(gd32f30x PUBLIC
    GD32F30X_HD                             # GD32F303RC 属于 HD (256K) 系列
    HXTAL_VALUE=8000000U                    # 板上 HSE 8MHz（与 system_gd32f30x.c 一致）
)

# vendor 库自身的 -Wno 抑制（只对 lib 生效，不影响应用代码）
target_compile_options(gd32f30x PRIVATE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-unused-variable
)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
file(GLOB APP_SRC CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/bsp/*.c"
)

add_executable(lvgl-ai-lab.elf
    ${APP_SRC}
    ${STARTUP_FILE}
)

target_include_directories(lvgl-ai-lab.elf PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bsp
)

target_link_libraries(lvgl-ai-lab.elf PRIVATE gd32f30x)

target_link_options(lvgl-ai-lab.elf PRIVATE
    -T${LINKER_FILE}
    -Wl,-Map=${CMAKE_BINARY_DIR}/lvgl-ai-lab.map,--cref
)

# 重新链接当 linker script 改时
set_target_properties(lvgl-ai-lab.elf PROPERTIES
    LINK_DEPENDS ${LINKER_FILE}
)

# ---------------------------------------------------------------------------
# Post-build: print size + produce .bin/.hex
# ---------------------------------------------------------------------------
add_custom_command(TARGET lvgl-ai-lab.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:lvgl-ai-lab.elf>
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:lvgl-ai-lab.elf> ${CMAKE_BINARY_DIR}/lvgl-ai-lab.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:lvgl-ai-lab.elf> ${CMAKE_BINARY_DIR}/lvgl-ai-lab.hex
    COMMENT "Sizes / .bin / .hex"
)
