#!/usr/bin/env node
// smoke-test.js — LVGL-AI-Lab smoke gate
//
// 阶段 1: sessions 完整性（每个触及 src/tests/tools 的非 merge commit 必须含 session: <slug> + pre/post 双件存在）
// 阶段 2: 构建 size 检查（占位 stub，CMake 工程进来后填）
// 阶段 3: 静态分析（占位 stub）
//
// Usage: node tests/smoke-test.js
// 退出码: 0 = PASS, 1 = FAIL

'use strict';

const { execSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');

const ROOT = path.resolve(__dirname, '..');
const SESS_DIR = path.join(ROOT, 'docs', '_meta', 'sessions');

// ============================================================================
// 阶段 1: sessions 完整性
// ============================================================================
function checkSessions() {
    console.log('=== 阶段 1: sessions 完整性 ===');

    // baseline: 项目 first commit 日期；git --since 用 UTC，多放一天容错
    const SINCE = '2026-05-01';

    // 取所有非 merge commit
    let log;
    try {
        log = execSync(
            `git log --since=${SINCE} --no-merges --format=%H%x09%s`,
            { cwd: ROOT, encoding: 'utf8' }
        );
    } catch (e) {
        console.error('  ERR: git log 失败:', e.message);
        return false;
    }

    const lines = log.trim().split('\n').filter(Boolean);
    if (lines.length === 0) {
        console.log('  （since=' + SINCE + ' 无 commit）');
        return true;
    }

    let violations = 0;
    let exempt = 0;
    let checked = 0;

    for (const line of lines) {
        const [sha, ...subjParts] = line.split('\t');
        const subject = subjParts.join('\t');

        // 取 commit 触及的文件列表
        const files = execSync(
            `git show --name-only --format= ${sha}`,
            { cwd: ROOT, encoding: 'utf8' }
        ).trim().split('\n').filter(Boolean);

        // 是否触及实现路径
        const touchesImpl = files.some(f =>
            f.startsWith('src/') ||
            f.startsWith('tests/') ||
            f.startsWith('tools/')
        );

        if (!touchesImpl) {
            exempt++;
            continue;
        }
        checked++;

        // 必须含 session: <slug>
        const m = subject.match(/session:\s*([a-z0-9-]+)/);
        if (!m) {
            console.error(`  FAIL [${sha.substring(0,7)}] 触及 src/tests/tools 但无 'session: <slug>': ${subject}`);
            violations++;
            continue;
        }

        const slug = m[1];
        // 找 pre/post 文件
        const sessFiles = fs.existsSync(SESS_DIR) ? fs.readdirSync(SESS_DIR) : [];
        const hasPre = sessFiles.some(f => f.endsWith(`-${slug}-pre.md`));
        const hasPost = sessFiles.some(f => f.endsWith(`-${slug}-post.md`));

        if (!hasPre) {
            console.error(`  FAIL [${sha.substring(0,7)}] session=${slug} 但无 *-${slug}-pre.md`);
            violations++;
            continue;
        }
        if (!hasPost) {
            console.error(`  FAIL [${sha.substring(0,7)}] session=${slug} 但无 *-${slug}-post.md`);
            violations++;
            continue;
        }

        console.log(`  PASS [${sha.substring(0,7)}] session=${slug}`);
    }

    console.log(`  ----`);
    console.log(`  ${checked} commits 检查 / ${exempt} 豁免（纯 docs）/ ${violations} violations`);
    return violations === 0;
}

// ============================================================================
// 阶段 2: 构建 size 检查（stub）
// ============================================================================
function checkBuildSize() {
    console.log('=== 阶段 2: 构建 size（stub）===');
    const elf = path.join(ROOT, 'build', 'lvgl-ai-lab.elf');
    if (!fs.existsSync(elf)) {
        console.log(`  SKIP: ${elf} 不存在（CMake 工程尚未建立或未编译）`);
        return true;
    }
    // 占位：未来用 arm-none-eabi-size 解析 + 阈值 (Flash<200K, RAM<40K)
    console.log('  TODO: arm-none-eabi-size 解析 + 阈值检查');
    return true;
}

// ============================================================================
// 阶段 3: 静态分析（stub）
// ============================================================================
function checkStaticAnalysis() {
    console.log('=== 阶段 3: 静态分析（stub）===');
    console.log('  TODO: -Wall -Wextra -Werror 在 CMake 工程进来后激活');
    return true;
}

// ============================================================================
// Main
// ============================================================================
const results = [
    checkSessions(),
    checkBuildSize(),
    checkStaticAnalysis(),
];

const allPass = results.every(Boolean);
console.log('');
console.log(allPass ? 'SMOKE: PASS' : 'SMOKE: FAIL');
process.exit(allPass ? 0 : 1);
