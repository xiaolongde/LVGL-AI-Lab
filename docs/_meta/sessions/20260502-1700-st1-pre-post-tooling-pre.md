---
type: session-pre
task: st1-pre-post-tooling
backlog_item: card1-minimal-debuggable
started: 2026-05-02T17:00+08:00
---

## 任务理解

建立 pre/post 自评工具 (`tools/session.sh`) + smoke-gate sessions 完整性检查 (`tests/smoke-test.js`)。本任务是方法论 infra，**必须最先完成**——后续所有 sub-task 的 B 路径硬约束（pre 不写不让动手 / post 不写不让 merge）需要零摩擦工具支持，否则纪律必崩。

**本 pre 文件是 bootstrap 手写**（工具尚未存在）——这是项目的元起点。后续所有 pre/post 都将走 `tools/session.sh`。

## 计划

1. **`tools/session.sh`**（bash，git-bash on Windows 兼容）
   - `new <slug>` → 生成 `YYYYMMDD-HHMM-<slug>-pre.md` 含 frontmatter + 三段模板
   - `close <slug>` → 找 matching pre（最新的）、提取时间戳前缀、建对应 `-post.md`
   - `list` → 列未 close 的 pre（pre 存在但 post 不存在）

2. **`tests/smoke-test.js`**（Node.js，与 smoke-gate skill 调用约定 `node tests/smoke-test.js` 一致）
   - 阶段 1: sessions 完整性
     - 取 `git log --since=<起跑日> --no-merges` 所有 commit
     - 对每条 commit：若 diff 触及 `src/` / `tests/` / `tools/` → 必须 commit msg 含 `session: <slug>` + 对应 `*-<slug>-pre.md` 和 `*-<slug>-post.md` 双件存在；纯 docs/ commit 豁免
     - violations 累加，最后 exit 1 if any
   - 阶段 2: 占位 stub（构建 size 检查 + 静态分析），等 CMake 工程进来再填
   - 输出: structured PASS/FAIL with reason for each commit

3. **`README.md`** 加 "Sessions discipline" 段（如何用 session.sh）

4. **Dogfood**: 用 session.sh close 写本任务的 post

5. **Commit**: 一个 commit 完成，message 含 `session: st1-pre-post-tooling`

## 卡点预判

- `date +%Y%m%d-%H%M` 在 git-bash 下行为应该 OK（GNU coreutils）
- `ls *-<slug>-pre.md` glob 在 git-bash 下 case-sensitive；强制 slug 小写
- Node.js on Windows: 已确认在 PATH 里 (`/c/Program Files/nodejs/node.exe`)
- `close` 找 pre 文件需 robust：用 `ls -t` 取最新（避免歧义；多 pre 同 slug 是 bug）
- smoke-test.js 不能扫整个 git history（性能 + 把 bootstrap commit 都算进去）—— 用 `git log --since=2026-05-02`（state.started）做 baseline
- "触及 src/tests/tools" 的判定：用 `git show --name-only`，过滤路径前缀
- commit msg 解析：`session:` 后接 slug，多 slug 用逗号或多行（先支持单 slug）

## 验收（自查清单）

- [ ] `tools/session.sh new test-foo` 在 docs/_meta/sessions/ 建出符合 schema 的 pre 文件
- [ ] `tools/session.sh close test-foo` 建对应 post 文件，frontmatter link 回 pre
- [ ] `tools/session.sh list` 列出未 close 的 pre
- [ ] `node tests/smoke-test.js` 在干净状态下 PASS
- [ ] 故意 commit 一个无 `session:` 标记的 src/ change，smoke-test FAIL
- [ ] dogfood: 本任务用 session.sh close 完成
