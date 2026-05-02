---
type: session-post
task: si-lvgl-fs-drv
pre: 20260502-2017-si-lvgl-fs-drv-pre.md
ended: 2026-05-02T20:49:18+08:00
outcome: success
---

## 实际遇到

预判 #1（Flash 紧）**严重命中**：第一次 build = 270K Flash，超 14K。

预判外坑：
- **`lv_fs_open` 返 LV_FS_RES_UNKNOWN (13)，root cause 不在 LVGL glue 而在 SD 卡内容**：先猜 FF_USE_LFN=0 让 f_open fail（是真因之一），开 LFN 后仍 NO_FILE → 加 dir listing → 才发现**根目录里根本没有 README.TXT**。sh 时存在的文件，到 si 测时已不在（用户操作或卡上下文丢失）。
- **gc-sections 不能完全替代 widget USE 开关**：以为 `LV_USE_CHART=1` 但 chart 未在 main.c 引用 = 链接器删掉。实测：从 270K → 227K（省 43K）靠的是手动关 LV_USE_* 开关，不是靠 gc-sections。LXO38R 第 3 次重现。
- **Release `-Os` 反而比 Debug `-Og` 大**：Debug 270K，Release 343K（同 source）。原因未深究，绕过——继续用 Debug。

## 修复路径

**Flash overflow 修复（按顺序）**：
1. 关 `LV_USE_CALENDAR/CHART/SPINBOX/...`（21 个未用 widget 全关）→ -43K → 227K ✓
2. `LV_LOG_LEVEL = NONE` + `LV_USE_OBJ_PROPERTY_NAME = 0`（小幅）
3. （试过：FF_USE_LFN=0 省 ~3K，但导致 f_open 找不到 LFN-only 文件 → 回退）

**lv_fs 测不到文件修复（三步硬件诊断 LXO38V 第 3 次应用）**：
1. **隔离**：在 lv_fs_open 之前调 direct `f_open("0:/README.TXT")` → 也是 FR_NO_FILE → 排除 lv_fs glue bug
2. **观察**：加 `f_opendir + f_readdir` 列根目录前 6 个文件 → 看到"根目录有 6 文件但无 README.TXT"
3. **绕过假设**：改主程序"读 readdir 返回的第一个 regular file"，不再假设固定文件名 → ✓ 通过

**最终验证**：picked 文件 + lv_fs_open=OK + lv_fs_read 数据显示 + tick label 持续递增 → **4 论据互证**。

Flash 231228 B / 256K (88.21%)，RAM 30984 B / 48K (63.04%)。

## 学到的（方法论素材）

**lesson 候选 1：gc-sections 是 last-resort，不是 first-line size 优化**

LVGL 这种"特性即代码"型库，正确顺序是：
1. **先**用 `LV_USE_*=0` 关掉不需要的 widget / feature（编译期裁剪）
2. **再**靠 gc-sections 删 dead 代码（链接期裁剪）

只靠 gc-sections，未引用的 widget 仍可能因为 dispatch table / theme bind / global init 而被链接进 .text。LXO38R 第 3 次重现（前两次：lv_conf 配置项隐式依赖、KIT vendor lib 完整 init 序列）→ **该升级到 P0 lesson**。

**lesson 候选 2：硬件 bring-up sign-off 需要"环境同质性"假设**

sh 验证 README.TXT 存在 → si 假设它仍在 → 实际不在。硬件实验环境（SD 卡内容、连线、上电顺序）跟代码不一样，**不会自动持续**。因此 sign-off 时要么：
- (a) 把环境状态写进 sign-off log（"sh 测时 SD 根目录: README.TXT (60B)"），下次复现要先 verify
- (b) 用"内容无关"的验证（这次走 readdir 拿第一个 file 就避免了这个问题）

应用 patch：硬件 bring-up 类任务的端到端测试，**优先选"内容无关"路径**（list-then-pick > assume-fixed-name）。

**lesson 候选 3：三步硬件诊断法的"列根目录"诊断技巧**

调试找不到文件类问题时：
- 先 `f_opendir + f_readdir` 列出实际看到的；不要在错误假设上调代码
- 这是 LXO38V 的具体应用——"先观察实际状态"而不是"基于假设调参"
- 类似的工具：网络问题用 `tcpdump`、MCU 用 GDB 看寄存器、FS 用 readdir 看实际目录

**lesson 候选 4：LVGL v9.5 `LV_USE_FS_FATFS` 自带驱动正常工作**

不用自写 lv_port_fs_sd.c。配置 3 个宏（USE/LETTER/PATH）+ 调 `lv_fs_fatfs_init()` 即可。LV_FS_FATFS_PATH 不带斜杠，让 lv_fs_fatfs.c 自己拼路径。**省 100-200 行 glue 代码**。

**autonomous mode 评估**：

- self-recommend 全部正确（LV_USE_FS_FATFS 走内置 / size 优化按 widget 优先 / 用 readdir 绕路径假设）
- 1 次反向 backtrack（FF_USE_LFN=0 → 1）成本 5 min，可接受
- BFS 没启用（错误次数 < 3 同模式）
- LTRYSL 应用得当
