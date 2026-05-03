---
type: session-pre
task: m1-pc-simulator
backlog_item: v0.3-pc-dev-pipeline
started: 2026-05-03T20:02:51+08:00
---

## 任务理解

v0.3 m1（v0.3 北极星之一）：让同源代码同时跑在 MCU 和 PC 上。PC 有 LVGL Windows 原生 backend (Win32)，开发循环 90s (MCU build+flash) → 5s (host build+run)。这是后续 m2/m3 的基础设施。

## 计划

1. CMake `option(BUILD_HOST_SIM)`：分支控制 MCU vs host
2. 抽 `src/hw.h`：hw_boot/hw_install_font/hw_lv_disp_init/hw_lv_fs_init/hw_font_path/hw_delay/hw_boot_log
3. `hw_mcu.c`（现有 MCU 代码 + #ifndef BUILD_HOST_SIM 守卫）+ `hw_host.c`（Win32 stub + lv_windows_create_display）
4. main.c 改为 hw_* API only（无任何平台特定调用）
5. lv_conf.h: 把 LV_USE_FS_FATFS / LV_USE_FS_STDIO / LV_USE_WINDOWS 等 host-vs-MCU 选项加 #ifndef 守卫，让 cmake target_compile_definitions 可覆盖
6. host build 用 MSYS2 mingw64 gcc (本机已有)
7. 验证：windowed 320×172 (zoom 200%) 跑 sk 3 主题

## 卡点预判

1. **没有 host 编译器**：解决 — MSYS2 mingw64 已装在 /c/msys64/，gcc 15.2.0
2. **lv_conf.h 写死 LV_USE_FS_FATFS=1 等**：要重构成 #ifndef 才能 cmake -D 覆盖
3. **lv_port_disp.c 是 MCU 专用**：通过 hw_lv_disp_init 抽象，host 用 lv_windows_create_display
4. **font 在 SD 卡 vs host 文件系统**：用 lv_fs_stdio + LV_FS_STDIO_LETTER='S'，让 "S:/" 在 host 解析到 cwd
5. **FATFS 只 MCU build**：CMake `if(BUILD_HOST_SIM) ... return() endif()` 跳过 vendor/sdio/fatfs

## 验收

- [ ] BUILD_HOST_SIM=ON 配置通过
- [ ] cmake --build 生成 lvgl-ai-lab-host.exe
- [ ] montserrat_28.bin 自动 copy 到 build_host/
- [ ] 启动后 Windows 窗口出现，3 主题轮播
- [ ] MCU build 不被破坏（回归测试）
- [ ] post 含 lessons 候选

## 元注释（autonomous mode）

- LVGL 自带 Windows backend > 用 SDL2（无需额外依赖）→ self-recommend
- hw_* API 设计：minimal surface area，不要漏抽象（漏一个就要回头）
- 卡点 #2 lv_conf.h 加守卫是低风险机械性改动，自决
