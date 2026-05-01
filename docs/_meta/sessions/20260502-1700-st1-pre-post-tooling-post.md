---
type: session-post
task: st1-pre-post-tooling
pre: 20260502-1700-st1-pre-post-tooling-pre.md
ended: 2026-05-02T01:04:40+08:00
outcome: success
---

## 实际遇到

按预判完成主结构（session.sh + smoke-test.js + README 段），但有 2 个意外坑：

1. **Node `execSync` 在 Windows 默认走 cmd.exe，不是 bash**——我最初写 `--format='%H%x09%s'`（单引号包 format 字符串）在 git-bash CLI 里 OK，但通过 Node execSync 派发时 cmd 不剥单引号，单引号被原样传给 git，导致 SHA 字符串变成 `'5b56062...`，下一步 `git show` 解析为 path 失败 (`fatal: ambiguous argument`)。
2. **`git log --since=YYYY-MM-DD` 用 UTC 解析**——本地 +08:00 凌晨的 commit 在 UTC 里是前一天，被 `--since=2026-05-02` 过滤掉了。需要把 SINCE 退一天到 `2026-05-01`，覆盖项目实际生命周期。

bash 测试 + Node 测试都各跑了一遍：session.sh new/list/close 走通；smoke-test.js 在 5 个 docs-only commit 上正确 PASS（0 violations / 5 豁免）。

## 修复路径

1. 单引号问题：把 `--format='%H%x09%s'` 改成 `--format=%H%x09%s`（无引号；git 接受），`git show --name-only --format=` 也已经无引号——OK
2. UTC 时区问题：把 SINCE 常量从 `'2026-05-02'` 改为 `'2026-05-01'`（项目首 commit 日期）

未来 alternative：用 `git log --since-as-filter` 配合 ISO 时间戳 + 本地 timezone 偏移，或直接用 `<baseline-sha>..HEAD` 范围。v0.1 不必。

## 学到的（方法论素材）

**这是项目第一份 post，已有 3 条可抽到 lessons 的素材**：

1. **跨平台 shell 假设是 AI 嵌入式开发的常见塌陷点**：Node `child_process.execSync` 在 Windows 走 cmd.exe，bash-style quoting 不可移植。Lessons 候选: `lessons/cross-platform-shell-quoting-traps.md`。

2. **git `--since` 用 UTC 不用 local time**：在 +0800 这种远离 UTC 的时区做日期过滤，需要给 baseline 多放一天。Lessons 候选: `lessons/git-since-utc-vs-local.md`。

3. **方法论 infra 的 dogfood 自指是可执行的设计验证**：本任务规定"pre 文件 bootstrap 手写、post 文件用工具写"，强制工具至少验证 close 路径。这种"自指"模式可推广到其他 infra 工具（Lessons 候选: `lessons/dogfood-as-bootstrap-test.md`）。

**Capability report 维度**: 工程 infra 层 + 跨平台脚本——AI 写出能跑的工具，但跨平台细节需要实测才能发现，初版必有 1-2 个隐形坑。这是个"AI 写脚本 demo 不能替代真跑"的 datapoint。
