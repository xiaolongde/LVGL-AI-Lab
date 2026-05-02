---
type: references-index
project: LVGL-AI-Lab
updated: 2026-05-01
---

# 项目资料库 — INDEX

> 本目录是 GD32F303RCT6 KIT 资料的**指针 + 提炼**，不复制原文。需要细节直接打开 KIT 源文件。
>
> KIT 路径：`D:\projects\GD\GD32F303RxT6 KIT\`

## 速查表

| 我想知道 | 看哪里 |
|---|---|
| MCU 关键参数 / 时钟树 / RAM-Flash 容量 | [`mcu.md`](mcu.md) |
| 板上接了什么 / 引脚映射全表 | [`board.md`](board.md) |
| 三块屏对比 / 选哪个 / 各 IC 关键参数 | [`screens.md`](screens.md) |
| 官方 SDK 怎么用 / Examples 哪些可抄 | [`sdk.md`](sdk.md) |
| Datasheet PDF / Reference Manual PDF | KIT/`4. DATASHEET/` |
| 板原理图 PDF | KIT/`2. HDK/V5.1/Schematic.pdf` |
| 烧录工具 / Keil / DFU | KIT/`1. TOOLS & DOC/` |

## KIT 源路径速查

```
D:\projects\GD\GD32F303RxT6 KIT\
├── 1. TOOLS & DOC/
│   ├── Keil/extracted/      ← GD32F30x_AddOn V2.2.0 已解压
│   ├── Keil/mdk525.exe      ← Keil MDK5.25 安装包（不用，我们走 CMake）
│   ├── GD32DfuDrivers...rar ← USB DFU 驱动（按需）
│   ├── GD32_MCU_HID_IAP_... ← HID IAP 上位机
│   └── USB_Virtual_Com_...  ← USB CDC 虚拟串口驱动
├── 2. HDK/V5.1/
│   ├── Schematic.pdf        ← 板原理图（关键！）
│   ├── Composite Drawing.pdf
│   └── 64PIN 底板 - V5.1.{Pcb,Sch}Lib  ← Altium 库
├── 3. SDK/extracted/GD32F303RCT6 Demo/
│   ├── Library/
│   │   ├── GD32F30x_Firmware_Library_V2.1.0/  ← 标准外设库
│   │   └── GD32F30x_AddOn_V2.1.0/             ← Keil pack 等价物
│   └── Examples/            ← 7 个工程模板
│       ├── LCD 0.96inch BOE/   ← ST7735S, 160x80
│       ├── LCD 1.14inch/       ← ST7789V3, 240x135
│       ├── LCD 1.47inch/       ← ?, 待查
│       ├── RGB LED/            ← 最简，hello-world 起点
│       ├── RTC/                ← LSE + RTC + BKP
│       ├── USART/              ← UART log
│       └── W25Qxx/             ← SPI flash (W25Q64 8MB on board)
└── 4. DATASHEET/
    ├── GD32F303xx_Datasheet_Rev1.8.pdf       ← 概览 + 引脚 + 电气参数 (~2.5MB)
    ├── GD32F30x_User_Manual_CN_Rev2.8.pdf    ← Reference Manual 中文版 (~13MB)
    └── 屏幕技术资料/
        ├── 0.96 inch/{0.96 寸 IPS 规格书.pdf, ST7735S 芯片手册.pdf}
        └── 1.14 inch/{1.14 寸 IPS 规格书.pdf, ST7789V3_SPEC...pdf}
```

## 我们为什么不直接用 Keil 工程

KIT 自带 7 个 MDK-ARM 工程，但本项目目标是 **CMake + OpenOCD + GDB** 闭环调试链。
迁移路径：抄 Keil 工程的 `Components/`、`Core/`、`GD32/` 三个源码目录 → 写 `CMakeLists.txt` 替换 `MDK-ARM/`，
linker script 自己写（参考 firmware lib 里的 `.s` 启动文件）。

## 烧录 + 调试 — ST-Link + pyOCD

用户实物烧录器: **ST-Link/V2**（VID 0x0483 PID 0x3748，SWD 接到 P1 header: SWDIO=PA13, SWDCLK=PA14, NRST, GND；3V3 不接，板子由自己 USB 供电避免双供电冲突）。

**实际工具**: pyOCD 0.44.0（pip 装到 user 目录，`python -m pyocd ...` 调用）。
- 装: `pip install --user pyocd`
- 探测 probe: `python -m pyocd list --probes`
- 列 builtin targets: `python -m pyocd list --targets`（`stm32f103rc` 是 builtin，覆盖 GD32F303RC）
- 烧录: `tools/flash.sh [build/lvgl-ai-lab.elf]` → `python -m pyocd flash -t stm32f103rc --erase chip <elf>`
- 调试: `tools/gdb-server.sh` 起 GDB server @ :3333，`arm-none-eabi-gdb -ex 'target extended-remote :3333' build/lvgl-ai-lab.elf` attach
- Pack 缓存：首次连接时 pyOCD 自动用 builtin SVD/算法；如需更新装 CMSIS pack 用 `python -m pyocd pack install stm32f103rc`

**为什么不用 OpenOCD**：原计划是 OpenOCD（references 老版本），但 Windows 上没装且 choco install 需 admin。pyOCD 用 pip 免 admin，原生 ST-Link 支持，3 秒装完。功能等价（GDB 协议层一样）。OpenOCD 留作未来 v0.2 候选（lessons 备录）。

**为什么 target 选 stm32f103rc**：GD32F303RxT6 在 SWD/Cortex-M4 寄存器层与 STM32F103 100% 兼容（GD 沿袭 F1 设计）。pyOCD builtin 的 `stm32f103rc` flash 算法 + memory map 与 GD32F303RC 一致（256K Flash @ 0x08000000，48K SRAM @ 0x20000000）。实测烧录 + 复位 + 读寄存器全通。

**Log 通道**：ST-Link 不支持 RTT（RTT 是 SEGGER J-Link 专有），走 **USART1 (PA9 TX, 115200 8N1)**，主机用 USB-TTL 串口工具或 putty/minicom 抓。（USART1 init 在 src/bsp/usart.c。）
- 备用 log: 板上 USB Type-C 走 USB CDC 虚拟串口（COM5/COM6 已枚举）—— v0.2 备选。

**接线注意**：板子如果通过 USB Type-C 自己供电，**ST-Link 上的 3V3 不要接**（避免双供电冲突）。只接 SWDIO/SWDCLK/GND/NRST 四线。

## 后续维护

- 资料库本身不会自动更新——AI 在 case 执行中读到新东西，写到对应文件下方"实践笔记"段
- 不重复 datasheet 内容，只放"读完后我知道但不在原文一句话内的事"
