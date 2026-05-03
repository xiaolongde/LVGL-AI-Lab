# LVGL-AI-Lab — top-level CMake (dual target: MCU + host PC simulator)
#
# MCU build (default):
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.toolchain.cmake
#   cmake --build build
#
# Host PC simulator build (LVGL Windows native backend, MinGW64):
#   cmake -B build_host -G Ninja -DBUILD_HOST_SIM=ON \
#         -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe
#   cmake --build build_host
#   ./build_host/lvgl-ai-lab-host.exe

cmake_minimum_required(VERSION 3.20)

option(BUILD_HOST_SIM "Build PC simulator (LVGL Windows backend) instead of MCU firmware" OFF)

if(BUILD_HOST_SIM)
    project(lvgl-ai-lab-host LANGUAGES C VERSION 0.1.0)
else()
    project(lvgl-ai-lab LANGUAGES C ASM VERSION 0.1.0)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ---------------------------------------------------------------------------
# Common paths
# ---------------------------------------------------------------------------
set(SRC_DIR     ${CMAKE_SOURCE_DIR}/src)
set(LVGL_DIR    ${CMAKE_SOURCE_DIR}/external/lvgl)
set(LV_PORT_DIR ${SRC_DIR}/lvgl_port)

# ---------------------------------------------------------------------------
# LVGL — common to both targets
# ---------------------------------------------------------------------------
file(GLOB_RECURSE LVGL_SRC CONFIGURE_DEPENDS
    "${LVGL_DIR}/src/*.c"
)

add_library(lvgl STATIC ${LVGL_SRC})

target_include_directories(lvgl PUBLIC
    ${LVGL_DIR}
    ${LV_PORT_DIR}
)

target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE=1
)

target_compile_options(lvgl PRIVATE
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-implicit-fallthrough
    -Wno-shift-negative-value
    -Wno-old-style-declaration
    -Wno-pointer-sign
    -Wno-type-limits
)

# =====================================================================
# HOST PC SIMULATOR build
# =====================================================================
if(BUILD_HOST_SIM)
    # tell lvgl + app to use Windows backend + stdio fs (NOT FATFS)
    target_compile_definitions(lvgl PUBLIC
        LV_USE_WINDOWS=1
        LV_USE_OS=LV_OS_WINDOWS
        LV_USE_FS_STDIO=1
        LV_USE_FS_FATFS=0
        LV_USE_TEXTAREA=1
        LV_USE_KEYBOARD=1
        LV_USE_BUTTONMATRIX=1
        # host: relax LV_MEM_SIZE so 5 binfonts (14/18/24/28/32) all fit
        LV_MEM_SIZE=131072
    )

    # link Win32 GUI libs into lvgl so the windows backend works
    target_link_libraries(lvgl PUBLIC user32 gdi32)

    # app source (no SD/FATFS/LCD code on host)
    file(GLOB THEMES_SRC CONFIGURE_DEPENDS "${SRC_DIR}/themes/*.c")
    set(HOST_APP_SRC
        ${SRC_DIR}/main.c
        ${SRC_DIR}/hw_host.c
        ${SRC_DIR}/desktop_logic.c
        ${THEMES_SRC}
    )

    add_executable(lvgl-ai-lab-host ${HOST_APP_SRC})
    target_compile_definitions(lvgl-ai-lab-host PRIVATE BUILD_HOST_SIM=1)
    target_include_directories(lvgl-ai-lab-host PRIVATE ${SRC_DIR})
    target_link_libraries(lvgl-ai-lab-host PRIVATE lvgl)

    # copy all SD assets (fonts + tstyle) next to the .exe so lv_fs_stdio finds them
    file(GLOB SD_ASSETS
        "${CMAKE_SOURCE_DIR}/tools/font_out/*.bin"
    )
    foreach(ASSET ${SD_ASSETS})
        get_filename_component(ASSET_NAME ${ASSET} NAME)
        add_custom_command(TARGET lvgl-ai-lab-host POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${ASSET}
                $<TARGET_FILE_DIR:lvgl-ai-lab-host>/${ASSET_NAME}
            COMMENT "SD asset: ${ASSET_NAME}"
        )
    endforeach()

    return()  # done — skip MCU section below
endif()

# =====================================================================
# MCU build (GD32F303RC + ST7789V3 + SD/FATFS)
# =====================================================================
set(VENDOR_DIR  ${CMAKE_SOURCE_DIR}/vendor/gd32f30x)
set(FATFS_DIR   ${CMAKE_SOURCE_DIR}/external/fatfs)
set(LINKER_FILE ${CMAKE_SOURCE_DIR}/linker/gd32f303rc.ld)
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/startup/startup_gd32f303rc.S)

# ---------------------------------------------------------------------------
# Vendor: GD32F30x firmware library
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
    ${SRC_DIR}
)

target_compile_definitions(gd32f30x PUBLIC
    GD32F30X_HD
    HXTAL_VALUE=8000000U
)

target_compile_options(gd32f30x PRIVATE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-unused-variable
)

# LVGL needs ff.h on the MCU build
target_include_directories(lvgl PUBLIC ${FATFS_DIR}/inc)

# ---------------------------------------------------------------------------
# FatFs R0.13c
# ---------------------------------------------------------------------------
file(GLOB FATFS_SRC CONFIGURE_DEPENDS
    "${FATFS_DIR}/src/*.c"
)

add_library(fatfs STATIC ${FATFS_SRC})
target_include_directories(fatfs PUBLIC ${FATFS_DIR}/inc)
target_link_libraries(fatfs PUBLIC gd32f30x)
target_compile_options(fatfs PRIVATE
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-implicit-fallthrough
    -Wno-old-style-declaration
)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
file(GLOB APP_SRC CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/bsp/*.c"
    "${SRC_DIR}/devices/*/*.c"
    "${SRC_DIR}/themes/*.c"
    "${LV_PORT_DIR}/*.c"
)
# remove host-only file from MCU build
list(FILTER APP_SRC EXCLUDE REGEX ".*hw_host\\.c$")

add_executable(lvgl-ai-lab.elf
    ${APP_SRC}
    ${STARTUP_FILE}
)

target_include_directories(lvgl-ai-lab.elf PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/bsp
    ${SRC_DIR}/devices/lcd
    ${SRC_DIR}/devices/sd
    ${LV_PORT_DIR}
)

target_link_libraries(lvgl-ai-lab.elf PRIVATE gd32f30x lvgl fatfs)

target_link_options(lvgl-ai-lab.elf PRIVATE
    -T${LINKER_FILE}
    -Wl,-Map=${CMAKE_BINARY_DIR}/lvgl-ai-lab.map,--cref
)

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
