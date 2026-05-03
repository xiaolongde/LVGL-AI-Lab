---
type: session-post
task: m1-pc-simulator
pre: 20260503-2002-m1-pc-simulator-pre.md
ended: 2026-05-03T21:31:55+08:00
outcome: success
---

## 实际遇到

预判全部命中（无意外）。一次 cmake reconfigure + 3 轮 build 修编译错就跑通：
1. 第一轮：`#error LV_OS_WINDOWS is required` → 加 LV_USE_OS=LV_OS_WINDOWS define + lv_conf 加 #ifndef 守卫
2. 第二轮：lv_textarea/lv_keyboard 未定义（lv_windows_input.c 引用） → 加 LV_USE_TEXTAREA/KEYBOARD/BUTTONMATRIX=1 + 守卫
3. 第三轮：hw_host.c include "drivers/windows/lv_windows_display.h" 找不到 → 改 "src/drivers/..." 因为 LVGL include path 是 ${LVGL_DIR} 不是 ${LVGL_DIR}/src

最终 build：`build_host/lvgl-ai-lab-host.exe` 3.1MB + montserrat_28.bin 16K 自动 copy。

**额外收获**：
- LVGL Windows native backend 比想象的稳：`lv_windows_create_display(L"title", w, h, zoom, dpi_override, sim_mode)` 一行起窗
- lv_fs_stdio + letter='S' + path="./" 完美映射 "S:/montserrat_28.bin" 到本地 file
- 200% zoom (640×344) 视觉上接近真实 1.47" 屏物理观感

## 修复路径

CMake 写法关键：`if(BUILD_HOST_SIM) ... return() endif()` 在 host 分支末尾用 return 跳过 MCU 段，避免引入 ASM 语言/ARM toolchain 检查。同 CMakeLists 双 target，无需复制。

lv_conf.h 守卫模式（每个 host-overridable 项）：
```c
#ifndef LV_USE_FS_FATFS
#define LV_USE_FS_FATFS 1
#endif
```
让 target_compile_definitions(lvgl PUBLIC LV_USE_FS_FATFS=0) 可以反转默认。

main.c 跨平台改造：所有平台特定调用走 `hw.h` API。MCU/host 各一份实现。

## 学到的（方法论素材）

**lesson 候选 1：嵌入式同源 dual-target = LVGL 项目的开发提速 30×**

MCU build+flash+reset+视觉验证 ≈ 90s。host build+run+视觉验证 ≈ 5s。同样的 widget 代码改一行 → 30s 看效果 vs 5s。**v0.1 的 13 次 user-in-the-loop 视觉介入大部分可以在 host 上自闭环**。设计模式：
- CMake option 控制 target
- hw.h 抽 platform 边界
- lv_conf.h #ifndef 守卫所有 host-override 选项
- 同源 .c 用 #ifdef BUILD_HOST_SIM 守 platform-only 代码

**lesson 候选 2：LVGL 的 lv_conf.h 必须设计为 "可被 cmake 覆盖"**

LVGL 默认 lv_conf.h 是"#define X 0"裸定义，无法 cmake -D 覆盖。改造为 #ifndef X #define X 0 #endif 后，target_compile_definitions 可以选择性反转。这是 LVGL 项目跨平台的必要前置改造。**早做早受益**：每加一个 LVGL feature 都顺手加守卫，避免日后批量改。

**lesson 候选 3：LVGL 的 Windows backend 比 SDL2 更轻**

LVGL v9.5 自带 lv_windows_*.c 用纯 Win32 (user32/gdi32)，无需 SDL2 依赖。Linux 党才用 SDL；Windows 直接 native。一行 `lv_windows_create_display` 起窗。

**autonomous mode 评估**：
- self-recommend "LVGL Windows backend > SDL2" → 正确（无外部依赖）
- self-recommend "lv_conf 加守卫 vs 开 lv_conf_host.h" → 正确（守卫局部修改，不分裂 config）
- 编译错三轮全是 LVGL 内部依赖揭示（OS / Textarea / Keyboard），不是设计错。机械性修。
