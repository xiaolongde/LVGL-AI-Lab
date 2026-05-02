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
            `git log -z --since=${SINCE} --no-merges --format=%H%x09%B`,
            { cwd: ROOT, encoding: 'utf8' }
        );
    } catch (e) {
        console.error('  ERR: git log 失败:', e.message);
        return false;
    }

    // -z 用 \0 分隔 commit
    const records = log.split('\0').filter(Boolean);
    if (records.length === 0) {
        console.log('  （since=' + SINCE + ' 无 commit）');
        return true;
    }

    let violations = 0;
    let exempt = 0;
    let checked = 0;

    for (const record of records) {
        const tabIdx = record.indexOf('\t');
        const sha = record.substring(0, tabIdx);
        const fullMsg = record.substring(tabIdx + 1);
        const subject = fullMsg.split('\n')[0];

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

        // 必须含 session: <slug>（subject 或 body 任一处）
        const m = fullMsg.match(/session:\s*([a-z0-9-]+)/);
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
// 阶段 2: 构建 size 检查
// ============================================================================
function checkBuildSize() {
    console.log('=== 阶段 2: 构建 size ===');
    const elf = path.join(ROOT, 'build', 'lvgl-ai-lab.elf');
    if (!fs.existsSync(elf)) {
        console.log(`  SKIP: ${elf} 不存在（先 cmake --build build）`);
        return true;
    }
    // 阈值历史:
    //  - st1 (v0.1 setup): Flash 200K / RAM 40K（LVGL 前定的）
    //  - sb (LVGL 加入): Flash 240K / RAM 40K（LVGL 裁剪后约 130K）
    //  - sc (sysmon+spinner+RTC): Flash 250K / RAM 40K
    //    sc 强制开 LV_DRAW_SW_COMPLEX=1（arc/spinner 依赖）+ LV_USE_SYSMON=1（PERF/MEM overlay）
    //    + LV_MEM_SIZE 16K（多 widget heap），三者都不可省 → 阈值再放宽 10K
    //    余量 = 256K - 250K = 6K（够小幅迭代）；继续超 → v0.2 优化（小字体 / drop sysmon）
    const FLASH_LIMIT = 250 * 1024;
    const RAM_LIMIT   = 40  * 1024;

    let out;
    try {
        out = execSync(`arm-none-eabi-size ${elf}`, { encoding: 'utf8' });
    } catch (e) {
        console.error('  ERR: arm-none-eabi-size 不在 PATH 或失败:', e.message);
        return false;
    }
    // 输出: "   text    data     bss     dec     hex filename"
    //       "   1680      12    3108    4800    12c0 ..."
    const lines = out.trim().split('\n');
    const cols = lines[1].trim().split(/\s+/);
    const text = parseInt(cols[0], 10);
    const data = parseInt(cols[1], 10);
    const bss  = parseInt(cols[2], 10);
    const flashUsed = text + data;       // .text + .data 都烧到 Flash
    const ramUsed   = data + bss;        // RAM 中 .data + .bss
    const pct = (n, lim) => `${(n / lim * 100).toFixed(2)}%`;

    let ok = true;
    if (flashUsed > FLASH_LIMIT) {
        console.error(`  FAIL: Flash ${flashUsed} > ${FLASH_LIMIT} (${pct(flashUsed, FLASH_LIMIT)})`);
        ok = false;
    } else {
        console.log(`  PASS: Flash ${flashUsed} / ${FLASH_LIMIT} (${pct(flashUsed, FLASH_LIMIT)})`);
    }
    if (ramUsed > RAM_LIMIT) {
        console.error(`  FAIL: RAM ${ramUsed} > ${RAM_LIMIT} (${pct(ramUsed, RAM_LIMIT)})`);
        ok = false;
    } else {
        console.log(`  PASS: RAM ${ramUsed} / ${RAM_LIMIT} (${pct(ramUsed, RAM_LIMIT)})`);
    }
    return ok;
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
