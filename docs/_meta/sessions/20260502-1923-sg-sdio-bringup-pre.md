---
type: session-pre
task: sg-sdio-bringup
backlog_item: card3-sd-fatfs-gui-resources
started: 2026-05-02T19:23:37+08:00
---

## 任务理解

v0.2 Card 3 第一站：让板上 SD 卡 SDIO 接口在硬件层 work。检测 SD 卡 + 读 CSD/CID + 显示 capacity 到屏。

**简化策略**：暂时弃 LVGL 用 KIT TFTLCD 直接 LCD_ShowString。Flash 约 80KB 留出大空间给 SDIO + future FATFS。LVGL 在 si (lv_fs_drv 步骤) 时再合回。

## 计划

1. **抄文件**：
   - `KIT/.../Library/GD32F30x_Firmware_Library_V2.1.0/Examples/SDIO/Read_write/sdcard.{c,h}` → `src/devices/sd/`
   - 文件 2448 + 262 行，是完整 SD card protocol stack（CMD0/8/2/3/9/7/55/41 + ACMD41 + CSD parse + block r/w）

2. **写 GPIO 配置** `src/bsp/sd_gpio.{c,h}`：
   - PC8-12 配 AF_PP 50MHz (SDIO_D0-3 + SDIO_CK)
   - PD2 配 AF_PP 50MHz (SDIO_CMD)
   - 启 RCU_SDIO + RCU_GPIOC + RCU_GPIOD 时钟

3. **改 main.c 为 SD bring-up test**：
   - 保留 systick / gpio / spi_lcd_init / LCD_Init / LCD_Clear（LVGL 全删）
   - 加 sd_gpio_init() + sd_init() + sd_card_information_get(&info)
   - LCD_ShowString 显示卡 type / capacity / RCA / status

4. **CMakeLists.txt** 加 `src/devices/sd/*.c` glob

5. **build → 验证 size 远低于 LVGL 版本** (估 ~80K vs LVGL 版 240K)

6. **硬件准备等用户**：要求 user 插 microSD 卡 (任何容量, FAT32 或 exFAT 都行) 然后告知"卡插了"

7. **flash + reset → 用户报屏内容**

## 卡点预判

1. **GPIO AF 配置不对**：GD32F303 SDIO 引脚需 GPIO_MODE_AF_PP 50MHz。漏配 → CMD 发不出。
2. **SDIO 时钟 prescaler**：sdcard.c 内部默认配多少需读，错速度 → response timeout。
3. **vendor lib SDIO_IRQHandler**：sdcard.c 是否用中断？polling 模式简单。看实现。
4. **5V vs 3.3V**：SD 卡需 3.3V 电源，schematic 确认 SD1 通过板上 LDO 接 3.3V。
5. **NVIC 配置**：如果 sdcard.c 走中断，gd32f30x_it.c 需加 SDIO_IRQHandler stub。

## 验收

- [ ] cmake build 通过（sdcard.c + sd_gpio.c）
- [ ] Flash < 100KB（无 LVGL 版）
- [ ] 用户插 SD 卡 + flash + reset
- [ ] 屏显示 "SD type / capacity"
- [ ] GDB attach 看 sd_cardinfo struct 合法
- [ ] post 含 1-2 lessons 候选

## 元注释（autonomous mode）

- 决策点 default-yes self-recommended（polling > interrupt 简单）
- ≥ 3 次失败 BFS 回 parent（Card 3 plan，备选 SPI mode SD）
