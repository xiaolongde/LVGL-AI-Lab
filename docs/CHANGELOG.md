# LVGL-AI-Lab CHANGELOG

## [Unreleased]
- 立项 + design v0 APPROVED + bootstrap 骨架
- v0.1 BACKLOG 重构为 2 张大卡（用户反馈"10 张拖沓"后简化）：Card 1 闭环调试系统 + Card 2 LVGL 表盘；卡内 sub-task 由 AI 自决
- KIT 资料解压 (D:\projects\GD\GD32F303RxT6 KIT\) + `docs/references/{INDEX, mcu, board, screens, sdk}.md` 资料库建立
- lessons/ + capability-report/ 目录预建（v0.1 retro 时填充）
- 工具链已钉死：CMake + arm-none-eabi-gcc + OpenOCD + GDB；烧录器 ST-Link；log USART1 PA9 TX 115200
- **Card 1 / st1**: pre/post 自评工具 `tools/session.sh` (new/close/list) + smoke gate `tests/smoke-test.js`（sessions 完整性检查阶段 1 启用，构建 size + 静态分析阶段 2/3 待激活）
