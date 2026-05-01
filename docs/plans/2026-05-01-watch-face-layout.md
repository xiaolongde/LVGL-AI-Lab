---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 静态表盘布局（time + date + 假电量图标）

## 意图
v0.1 终极交付：一个真正像"手表"的静态表盘——大字体时间居中、上方日期文本、下方电量条/图标（假数据）。这一步决定 v0.1 demo 视觉是否能让用户/外部观众看一眼就懂"这是个手表"。

方法论价值：AI 在"组合多个 widget + 视觉层次 + 屏幕空间预算"这件事上的能力——是按 LVGL flex/grid 排还是 abs 坐标硬怼。

## 验收
- ✅ `src/app/screen_main.c/h` 创建表盘屏：date label（上）+ time widget（中）+ battery indicator（下）
- ✅ 字号层次：time 最大（如 36px）、date 中（如 14px）、battery 标签小（如 12px）
- ✅ 电量用一个 lv_bar 假设 75%，配 "75%" 文本——v0.1 是假数据但视觉位置占好
- ✅ 实机看 demo：1 分钟连续运行，时间走 + 整体不抖
- ✅ qa-report：录视频 + 截图（用相机翻拍即可）+ "看一眼能否识别为手表"自评打分
- ✅ session post：layout 写法选择（flex / grid / abs）+ 字体生成的麻烦点

## 计划步骤
1. session pre：AI 草绘 layout（ASCII 即可），决定用 lv_obj_set_style + flex 还是 abs
2. 实现 screen_main.c
3. 选字体（time 36px 可能要内嵌）—— 评估 Flash 余量
4. 接入 date label（先硬编码 "2026-05-01"，校时留 v0.2）
5. 接入 battery bar（假数据 75%）
6. 编 + size 看 Flash 留有余量
7. 烧录 + 实机看 1 分钟
8. 录视频 + 拍照 + 自评分
9. session post
10. commit

## 风险 / Open Questions
- 字体 36px 内嵌可能让 Flash 占用激增（每个数字字符 ~500B-1KB）—— 可考虑只内嵌 0-9 + ":" 共 11 字符
- partial buffer 5KB 在大屏 320x240 上重绘大字体可能多次 flush——观察实机帧率
- "看一眼像手表吗"是主观打分，retro 时拉用户 + 1-2 个外部人盲测更客观

## 依赖
- [[2026-05-01-time-widget]]

## 方法论 hook
- Lessons 章节: `lessons/lvgl-watch-face-layout.md`
- Capability report 维度: LVGL 应用层（视觉层次 + 字体管理）
