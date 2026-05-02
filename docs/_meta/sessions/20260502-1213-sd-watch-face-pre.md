---
type: session-pre
task: sd-watch-face
backlog_item: card2-lvgl-watch-face
started: 2026-05-02T12:13:35+08:00
---

## 任务理解

把 sb/sc 临时 stress demo（左上时间 + 中央 stress spinner）替换为**真正像手表**的表盘 layout。Card 2 v0.1 视觉收口。

目标：屏上一眼能识别为"这是个手表"——大字时间 + 顶部日期 + 底部状态条。pre 应用 LTIRB3 daily MVP 原则——一次到位的最小可见 demo。

## 计划

### 配置
- **关 LV_USE_SYSMON**（省 ~6KB Flash + 释放屏角空间给真 layout） — sysmon overlay 是 dev 期 stress test 用，final demo 不要
- **开 LV_FONT_MONTSERRAT_28**（大字时间 ~8KB Flash） — 当前只 14px
- 估总 Flash 变化：-6 +8 = +2KB → 244K，仍在 250K 阈值内

### Layout（320 × 172 横屏）
```
┌─────────────────────────────────────┐
│ 2026-05-02   Wed         [▓▓▓░] 75% │  顶端：日期 + 电量
├─────────────────────────────────────┤
│                                     │
│         12:34:56 (大字 28px)        │  中央：时间
│                                     │
├─────────────────────────────────────┤
│ ●●●●●●●●●●●●●●●●●●●●░░░░░░░░░░░░░  │  底端：秒进度条
└─────────────────────────────────────┘
```

### 步骤
1. lv_conf.h: `LV_USE_SYSMON 1 → 0` + `LV_FONT_MONTSERRAT_28 0 → 1`
2. main.c 重写 widget tree（保留 RTC + second_timer_cb 不动）：
   - top-left date label（小字 14px）
   - top-right battery label "75%" + 简易 bar（~24x6 px 框）
   - center 大字时间 lv_label_set_style_text_font(font_montserrat_28)
   - bottom 秒进度条（保留）
3. 删 stress spinner（已记入 sc post 的 FPS 数据点）
4. build/flash/reset
5. session post 含 watchface 视觉自评 1-2 条 lesson

## 卡点预判

1. **Montserrat 28 编进 Flash 后超 250K** → BFS backtrack: 改 Montserrat 24（小一档，~5KB） → 还超就只内嵌 0-9 + ":" 共 11 字符 (~1.5KB)
2. **关 sysmon 后某 LVGL 内部依赖坏掉**（不太可能，sysmon 是 optional） → 编译试一下
3. **大字时间渲染 partial buffer 拼接 visual artifact**（之前疑似 banner 14px 有这种）→ 28px 字符更高，更容易跨 chunk → fallback: partial buffer 8K → 12K
4. **layout 居中算位错** → LVGL flex/grid 或 hardcode pos，先 hardcode

## 验收

- [ ] cmake --build pass
- [ ] flash + reset → 屏视觉是"像手表"（一眼可识别）
- [ ] 时间走、秒进度条走、日期/电量静态可读
- [ ] 用户视觉 sign-off
- [ ] post 含 ≥ 2 lessons 候选

## 元注释（autonomous mode 启用，2026-05-02）

- 决策点 AI 直接选 self-recommended（如字体大小卡 size → 自动降级）
- ≥ 3 次失败回 parent BFS（sd 失败 → 回 Card 2 plan 看其他 layout 方案）
- 不再问 "继续吗"，直接做
