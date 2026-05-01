#!/usr/bin/env bash
# session.sh — pre/post 自评工具（B 路径双轨记录强方法论）
#
# Usage:
#   tools/session.sh new   <slug>    # 创建 pre 文件骨架
#   tools/session.sh close <slug>    # 创建 matching post 文件
#   tools/session.sh list            # 列出未 close 的 pre
#
# 文件命名: docs/_meta/sessions/YYYYMMDD-HHMM-<slug>-{pre,post}.md
#
# slug 规则: 小写 / kebab-case / 不含空格

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SESS_DIR="$ROOT/docs/_meta/sessions"

usage() {
    sed -n '3,11p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
    exit 1
}

[[ $# -lt 1 ]] && usage

cmd="$1"; shift || true

case "$cmd" in
    new)
        [[ $# -ne 1 ]] && { echo "ERR: new 需要 1 个参数 <slug>" >&2; exit 1; }
        slug="$1"
        if [[ ! "$slug" =~ ^[a-z0-9-]+$ ]]; then
            echo "ERR: slug 必须是小写 / kebab-case / 不含空格" >&2; exit 1
        fi
        mkdir -p "$SESS_DIR"
        ts=$(date +%Y%m%d-%H%M)
        path="$SESS_DIR/${ts}-${slug}-pre.md"
        if [[ -e "$path" ]]; then
            echo "ERR: 已存在 $path（同一分钟同 slug 二次 new）" >&2; exit 1
        fi
        cat > "$path" <<EOF
---
type: session-pre
task: ${slug}
backlog_item: ${BACKLOG_ITEM:-tbd}
started: $(date -Iseconds)
---

## 任务理解
<我以为这件事要做什么>

## 计划
<打算分几步、用什么方案>

## 卡点预判
<我猜哪几步会卡，预备的 plan B>
EOF
        echo "Created: $path"
        echo "Slug:    $slug"
        echo "Use:     tools/session.sh close $slug"
        ;;

    close)
        [[ $# -ne 1 ]] && { echo "ERR: close 需要 1 个参数 <slug>" >&2; exit 1; }
        slug="$1"
        # 找最新的 matching pre
        pre=$(ls -t "$SESS_DIR"/*-"${slug}"-pre.md 2>/dev/null | head -1 || true)
        if [[ -z "$pre" ]]; then
            echo "ERR: 找不到 *-${slug}-pre.md（先 tools/session.sh new $slug）" >&2; exit 1
        fi
        # 检查是否已 close
        ts_part=$(basename "$pre" | sed -E "s/-${slug}-pre\.md$//")
        post="$SESS_DIR/${ts_part}-${slug}-post.md"
        if [[ -e "$post" ]]; then
            echo "ERR: 已 close: $post" >&2; exit 1
        fi
        cat > "$post" <<EOF
---
type: session-post
task: ${slug}
pre: ${ts_part}-${slug}-pre.md
ended: $(date -Iseconds)
outcome: success
---

## 实际遇到
<跟预判的差别、意料外的事>

## 修复路径
<怎么解决的；如果没解决，卡在哪>

## 学到的（方法论素材）
<这次经验里能抽到 lessons / capability-report 的点是什么>
EOF
        echo "Created: $post"
        echo "Pre:     $pre"
        echo "记得编辑 outcome 字段为 success | partial | failed"
        ;;

    list)
        if [[ ! -d "$SESS_DIR" ]]; then
            echo "（无 sessions 目录）"; exit 0
        fi
        unclosed=0
        # 列所有 pre 文件，检查是否有对应 post
        while IFS= read -r pre; do
            base=$(basename "$pre")
            ts_slug="${base%-pre.md}"
            post="$SESS_DIR/${ts_slug}-post.md"
            if [[ ! -e "$post" ]]; then
                echo "  [unclosed] $base"
                unclosed=$((unclosed + 1))
            fi
        done < <(find "$SESS_DIR" -maxdepth 1 -name '*-pre.md' | sort)
        if [[ $unclosed -eq 0 ]]; then
            echo "（无未 close 的 pre）"
        else
            echo ""
            echo "$unclosed 个未 close 的 pre。用 tools/session.sh close <slug> 关闭。"
            exit 1
        fi
        ;;

    *)
        echo "ERR: 未知命令: $cmd" >&2
        usage
        ;;
esac
