---
type: session-post
task: sg-sdio-bringup
pre: 20260502-1923-sg-sdio-bringup-pre.md
ended: 2026-05-02T19:31:32+08:00
outcome: success
---

## 实际遇到

**一次跑通**。所有 5 个预判卡点全部 N/A：

1. **GPIO AF 配置不对** — N/A：KIT sdcard.c **自带 GPIO init**（line 2360-2362：PC8-12 + PD2 全配 AF_PP 50MHz），用户层不用写 sd_gpio.c。原 plan 步骤 2 撤销。
2. **SDIO clock prescaler** — N/A：sdcard.c 内部把 SDIO_CK 配为 24MHz（120/5）满足 SD spec ≤25MHz，不需调。
3. **vendor lib 中断模式** — N/A：sd_init() 默认 polling，不依赖 NVIC，gd32f30x_it.c 不用动。
4. **5V vs 3.3V** — N/A：板上 LDO 已通 3.3V 给 SD slot。
5. **NVIC 配置** — N/A：polling 模式不挂中断。

**实际执行**：
- 抄 `sdcard.{c,h}` 到 `src/devices/sd/`
- main.c 删 LVGL 全部代码 + 改写为 SD bring-up 测试程序
- CMakeLists.txt include path 加 `${SRC_DIR}/devices/sd`（GLOB `devices/*/*.c` 已覆盖 .c 抄入）
- build：Flash 28436 B / 256K (10.85%)，RAM 5232 B / 48K (10.64%)
- flash + reset → 屏显示 "SD OK!" + 卡 Type/Capacity/RCA/CID
- 用户 "SD OK" sign-off

## 修复路径

无故障。从 plan 到 sign-off 一遍过，估计实际 ~30 min（含 KIT 资料检索 + build + flash）。

## 学到的（方法论素材）

**lessons 候选**（待 retro 沉淀）：

1. **vendor 资料库 self-contained 性质常被低估**：
   - 写 plan 时假定要"写 sd_gpio.c"，实际 KIT sdcard.c 是完整的 driver（包含 RCU + GPIO + SDIO + protocol stack 全栈）
   - **方法论**：抄 vendor 文件前**先 grep "_init"** 看 init 函数全部依赖；如果 init 函数自含 RCU/GPIO，则用户层零代码。
   - 这次 plan 的 6 步缩到 3 步实际工作量。

2. **多论据原则的反例（成功侧）**：
   - 屏上 "SD OK!" 是 1 个 datapoint
   - 但同时显示了 capacity/type/RCA/CID 4 个独立信息源
   - **如果只显示 "SD OK"，仍可能 CMD0 假阳性（vendor 库返回 SD_OK 但卡没真初始化）**；CID 显示是**强论据**：能读 CID = CMD2 走通 = identification phase 完成
   - 应用 LTIRBE：性能/正确性 claim 都要 ≥ 2 个互相印证

3. **autonomous mode + default-yes self-recommended 验证成功**：
   - plan 阶段我自决"polling > interrupt"，没问用户
   - 节省 ~5 min 用户介入，**结果正确**
   - LTRYSL 应用：在低风险点（vendor 库默认值）上 self-recommend 是合理的

4. **Flash budget 真相**：
   - LVGL 版（Card 2 sd 收尾）241K / 256K = 94.1%
   - 删 LVGL 后 28K / 256K = 10.9%
   - **剩 228K 给 FATFS + lv_fs_drv + GUI 资源**：充足
   - 后续 sh (FATFS ~20-30K) + si (LVGL 回归 + lv_fs_drv ~5K) → ~270-280K，超 256K → **必须靠 SD 加载 GUI 资源（图片/字体）才能装下大字体表盘**
