#!/usr/bin/env python3
"""
Comprehensive Performance and RAM Consumption Benchmark Suite
Comparing Clang/Clang++, GCC/G++, and TCC/T++ across optimization flags and architectures.
Generates an interactive standalone HTML report and JSON summary.
"""

import os
import sys
import time
import json
import argparse
import subprocess
import shutil
import platform
import glob
import threading
import concurrent.futures

# Try to find /usr/bin/time or time binary
TIME_BIN = shutil.which("time")
if not TIME_BIN or not os.path.exists("/usr/bin/time"):
    TIME_BIN = "/usr/bin/time" if os.path.exists("/usr/bin/time") else None

def get_binary_size(path):
    try:
        return os.path.getsize(path)
    except Exception:
        return 0

def run_command_measured(cmd, build_dir, timeout_sec=60):
    """
    Run command using /usr/bin/time to accurately capture real elapsed time and peak memory (Max RSS).
    Returns (returncode, elapsed_ms, peak_rss_kb, stdout, stderr)
    """
    tid = threading.get_ident()
    time_output_file = os.path.join(build_dir, f"bench_time_{os.getpid()}_{tid}_{time.time_ns()}.txt")
    
    if TIME_BIN and os.path.exists(TIME_BIN):
        # %M = max RSS in KB, %e = elapsed real seconds, %U = user sec, %S = sys sec
        time_cmd = [TIME_BIN, "-f", "TIME_METRICS: max_rss_kb:%M elapsed_sec:%e user_sec:%U sys_sec:%S", "-o", time_output_file] + cmd
    else:
        time_cmd = cmd

    start_t = time.perf_counter()
    try:
        proc = subprocess.Popen(time_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = proc.communicate(timeout=timeout_sec)
        end_t = time.perf_counter()
        rc = proc.returncode
        wall_ms = (end_t - start_t) * 1000.0
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, stderr = proc.communicate()
        if os.path.exists(time_output_file):
            os.remove(time_output_file)
        return -1, timeout_sec * 1000.0, 0, stdout, "TIMEOUT"
    except Exception as e:
        if os.path.exists(time_output_file):
            os.remove(time_output_file)
        return -1, 0, 0, "", str(e)

    peak_rss_kb = 0
    elapsed_ms = wall_ms

    if os.path.exists(time_output_file):
        try:
            with open(time_output_file, "r") as f:
                content = f.read()
            for line in content.splitlines():
                if "TIME_METRICS:" in line:
                    parts = line.split()
                    for p in parts:
                        if p.startswith("max_rss_kb:"):
                            peak_rss_kb = int(p.split(":")[1])
                        elif p.startswith("elapsed_sec:"):
                            sec = float(p.split(":")[1])
                            elapsed_ms = sec * 1000.0
            os.remove(time_output_file)
        except Exception:
            pass

    return rc, round(elapsed_ms, 2), peak_rss_kb, stdout, stderr

def discover_compilers(build_dir, tcc_path=None):
    """
    Find available compilers on the system.
    """
    compilers = {}
    
    # GCC / G++
    gcc_path = shutil.which("gcc")
    gpp_path = shutil.which("g++")
    if gcc_path: compilers["gcc"] = gcc_path
    if gpp_path: compilers["g++"] = gpp_path

    # Clang / Clang++
    clang_path = shutil.which("clang")
    clangpp_path = shutil.which("clang++")
    if clang_path: compilers["clang"] = clang_path
    if clangpp_path: compilers["clang++"] = clangpp_path

    # TCC / T++ (x86_64, i386, arm, arm64, riscv64)
    if not tcc_path or not os.path.exists(tcc_path):
        tcc_path = os.path.join(build_dir, "tcc")
    if os.path.exists(tcc_path):
        compilers["tcc"] = tcc_path
        compilers["t++"] = tcc_path # TCC compiles C++ in cpp mode

    # Cross TCC targets
    for arch in ["x86_64", "i386", "arm", "arm64", "riscv64"]:
        arch_tcc = os.path.join(build_dir, f"{arch}-tcc")
        if os.path.exists(arch_tcc):
            compilers[f"tcc-{arch}"] = arch_tcc

    return compilers

def discover_emulators():
    emulators = {}
    for arch, bin_name in [("arm", "qemu-arm"), ("arm64", "qemu-aarch64"), ("riscv64", "qemu-riscv64"), ("i386", "qemu-i386")]:
        p = shutil.which(bin_name)
        if p:
            emulators[arch] = p
    return emulators

def build_and_run_benchmark(bench_file, lang, compiler_name, compiler_bin, opt_flag, arch, build_dir, emulators, compile_runs=3, exec_runs=5, timeout_sec=60):
    """
    Compile and run a single benchmark configuration over multiple iterations for statistical stability.
    """
    bench_basename = os.path.splitext(os.path.basename(bench_file))[0]
    tid = threading.get_ident()
    out_exe = os.path.join(build_dir, f"bench_{bench_basename}_{compiler_name}_{opt_flag.replace('-', '')}_{arch}_{tid}.exe")
    
    # Construct compilation command
    compile_cmd = []
    tcc_include_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "include"))
    tcc_bdir = os.path.abspath(build_dir)
    
    is_tcc = "tcc" in compiler_name or "t++" in compiler_name
    math_needed = any(k in bench_basename for k in ("fft", "mandelbrot", "nbody", "raytracer", "spectral_norm"))
    
    if is_tcc:
        compile_cmd = [compiler_bin, "-B", tcc_bdir, "-I", tcc_include_dir]
        if arch == "x86_64":
            if os.path.exists("/usr/lib/x86_64-linux-gnu"):
                compile_cmd += ["-L/usr/lib/x86_64-linux-gnu"]
        if opt_flag != "-O0":
            compile_cmd += [opt_flag]
        compile_cmd += ["-o", out_exe, bench_file]
        if math_needed:
            compile_cmd += ["-lm"]
    else:
        compile_cmd = [compiler_bin, opt_flag]
        if arch == "i386":
            compile_cmd += ["-m32"]
        elif arch == "arm" and "clang" in compiler_name:
            compile_cmd += ["--target=arm-linux-gnueabi"]
        elif arch == "arm64" and "clang" in compiler_name:
            compile_cmd += ["--target=aarch64-linux-gnu"]
        elif arch == "riscv64" and "clang" in compiler_name:
            compile_cmd += ["--target=riscv64-linux-gnu"]
        compile_cmd += ["-o", out_exe, bench_file]
        if math_needed:
            compile_cmd += ["-lm"]

    # Measure compilation over multiple runs
    c_times = []
    c_rss_list = []
    c_rc = 0
    c_stdout, c_stderr = "", ""

    for _ in range(max(1, compile_runs)):
        rc, t_ms, rss_kb, out, err = run_command_measured(compile_cmd, build_dir, timeout_sec=timeout_sec)
        c_rc = rc
        c_stdout = out
        c_stderr = err
        if rc != 0:
            break
        c_times.append(t_ms)
        c_rss_list.append(rss_kb)
    
    bin_size = get_binary_size(out_exe) if c_rc == 0 else 0
    
    if c_rc != 0 or len(c_times) == 0:
        if os.path.exists(out_exe):
            try: os.remove(out_exe)
            except Exception: pass
        return {
            "benchmark": bench_basename,
            "lang": lang,
            "compiler": compiler_name,
            "opt": opt_flag,
            "arch": arch,
            "compile_success": False,
            "compile_time_ms": c_times[0] if c_times else 0,
            "compiler_peak_rss_kb": max(c_rss_list) if c_rss_list else 0,
            "exec_success": False,
            "exec_time_ms": 0,
            "exec_peak_rss_kb": 0,
            "binary_size_bytes": 0,
            "compile_runs": len(c_times),
            "exec_runs": 0,
            "error": f"Compile failed (rc={c_rc}): {c_stderr}"
        }

    c_time_ms = sum(c_times) / len(c_times)
    c_rss_kb = max(c_rss_list)

    # Execution command
    exec_cmd = []
    if arch == "x86_64" or (arch == "i386" and platform.machine() in ("x86_64", "i386", "i686")):
        exec_cmd = [out_exe]
    elif arch in emulators:
        exec_cmd = [emulators[arch], out_exe]
    else:
        # Cannot run cross-binary without emulator
        if os.path.exists(out_exe):
            try: os.remove(out_exe)
            except Exception: pass
        return {
            "benchmark": bench_basename,
            "lang": lang,
            "compiler": compiler_name,
            "opt": opt_flag,
            "arch": arch,
            "compile_success": True,
            "compile_time_ms": c_time_ms,
            "compiler_peak_rss_kb": c_rss_kb,
            "exec_success": True,
            "exec_time_ms": 0, # Not executed (cross target without emulator)
            "exec_peak_rss_kb": 0,
            "binary_size_bytes": bin_size,
            "compile_runs": len(c_times),
            "exec_runs": 0,
            "note": f"Compiled for {arch} (no emulator)"
        }

    # Run execution over multiple iterations for statistical stability
    e_times = []
    e_rss_list = []
    e_rc = 0
    e_stdout, e_stderr = "", ""

    for _ in range(max(1, exec_runs)):
        rc, t_ms, rss_kb, out, err = run_command_measured(exec_cmd, build_dir, timeout_sec=timeout_sec)
        e_rc = rc
        e_stdout = out
        e_stderr = err
        if rc != 0:
            break
        e_times.append(t_ms)
        e_rss_list.append(rss_kb)
    
    # Cleanup executable
    if os.path.exists(out_exe):
        try:
            os.remove(out_exe)
        except Exception:
            pass

    if e_rc != 0 or len(e_times) == 0:
        return {
            "benchmark": bench_basename,
            "lang": lang,
            "compiler": compiler_name,
            "opt": opt_flag,
            "arch": arch,
            "compile_success": True,
            "compile_time_ms": c_time_ms,
            "compiler_peak_rss_kb": c_rss_kb,
            "exec_success": False,
            "exec_time_ms": 0,
            "exec_peak_rss_kb": 0,
            "binary_size_bytes": bin_size,
            "compile_runs": len(c_times),
            "exec_runs": len(e_times),
            "error": f"Execution failed (rc={e_rc}): {e_stderr}"
        }

    e_time_ms = sum(e_times) / len(e_times)
    e_rss_kb = max(e_rss_list)
    min_e_time_ms = min(e_times)

    return {
        "benchmark": bench_basename,
        "lang": lang,
        "compiler": compiler_name,
        "opt": opt_flag,
        "arch": arch,
        "compile_success": (c_rc == 0),
        "compile_time_ms": round(c_time_ms, 2),
        "compiler_peak_rss_kb": c_rss_kb,
        "exec_success": (e_rc == 0),
        "exec_time_ms": round(e_time_ms, 2),
        "exec_min_time_ms": round(min_e_time_ms, 2),
        "exec_peak_rss_kb": e_rss_kb,
        "binary_size_bytes": bin_size,
        "compile_runs": len(c_times),
        "exec_runs": len(e_times),
        "output_sample": e_stdout.strip().splitlines()[-1] if e_stdout.strip() else ""
    }

def generate_html_report(results, report_file):
    """
    Generate an interactive, visually rich HTML report.
    """
    results_json_str = json.dumps(results, indent=2)
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Compiler Benchmark Report: Clang/GCC/TCC & Clang++/G++/T++</title>
    <style>
        :root {{
            --bg-main: #0f172a;
            --bg-card: #1e293b;
            --bg-card-hover: #334155;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border: #334155;
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.2);
            --accent-green: #4ade80;
            --accent-yellow: #facc15;
            --accent-red: #f87171;
            --accent-purple: #c084fc;
        }}
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg-main);
            color: var(--text-main);
            line-height: 1.5;
            padding: 24px;
        }}
        .container {{
            max-width: 1400px;
            margin: 0 auto;
        }}
        header {{
            margin-bottom: 28px;
            padding-bottom: 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 16px;
        }}
        h1 {{
            font-size: 26px;
            font-weight: 700;
            color: var(--primary);
            display: flex;
            align-items: center;
            gap: 10px;
        }}
        .subtitle {{
            color: var(--text-muted);
            font-size: 14px;
            margin-top: 4px;
        }}
        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
            gap: 16px;
            margin-bottom: 28px;
        }}
        .stat-card {{
            background-color: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 4px 6px -1px rgba(0,0,0,0.1);
        }}
        .stat-card .label {{
            font-size: 13px;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }}
        .stat-card .value {{
            font-size: 28px;
            font-weight: 700;
            color: var(--primary);
            margin: 8px 0 4px 0;
        }}
        .stat-card .desc {{
            font-size: 12px;
            color: var(--accent-green);
        }}
        .controls-card {{
            background-color: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 16px;
            margin-bottom: 24px;
            display: flex;
            flex-wrap: wrap;
            gap: 16px;
            align-items: center;
        }}
        .control-group {{
            display: flex;
            align-items: center;
            gap: 8px;
        }}
        .control-group label {{
            font-size: 13px;
            color: var(--text-muted);
            font-weight: 600;
        }}
        select, input {{
            background-color: var(--bg-main);
            color: var(--text-main);
            border: 1px solid var(--border);
            padding: 6px 12px;
            border-radius: 6px;
            font-size: 13px;
            outline: none;
        }}
        select:focus, input:focus {{
            border-color: var(--primary);
        }}
        .charts-section {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(600px, 1fr));
            gap: 20px;
            margin-bottom: 28px;
        }}
        @media (max-width: 768px) {{
            .charts-section {{
                grid-template-columns: 1fr;
            }}
        }}
        .chart-card {{
            background-color: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 20px;
        }}
        .chart-card h3 {{
            font-size: 16px;
            margin-bottom: 16px;
            color: var(--text-main);
            display: flex;
            justify-content: space-between;
        }}
        .table-card {{
            background-color: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            overflow: hidden;
            margin-bottom: 28px;
        }}
        .table-header {{
            padding: 16px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid var(--border);
        }}
        .table-header h3 {{
            font-size: 16px;
        }}
        .table-responsive {{
            overflow-x: auto;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            font-size: 13px;
            text-align: left;
        }}
        th, td {{
            padding: 12px 16px;
            border-bottom: 1px solid var(--border);
        }}
        th {{
            background-color: rgba(15, 23, 42, 0.6);
            color: var(--text-muted);
            font-weight: 600;
            cursor: pointer;
            user-select: none;
        }}
        th:hover {{
            color: var(--primary);
        }}
        tr:hover {{
            background-color: var(--bg-card-hover);
        }}
        .badge {{
            display: inline-block;
            padding: 2px 8px;
            border-radius: 4px;
            font-size: 11px;
            font-weight: 600;
            text-transform: uppercase;
        }}
        .badge-tcc {{ background-color: rgba(56, 189, 248, 0.2); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.4); }}
        .badge-gcc {{ background-color: rgba(248, 113, 113, 0.2); color: #f87171; border: 1px solid rgba(248, 113, 113, 0.4); }}
        .badge-clang {{ background-color: rgba(192, 132, 252, 0.2); color: #c084fc; border: 1px solid rgba(192, 132, 252, 0.4); }}
        .badge-c {{ background-color: rgba(74, 222, 128, 0.2); color: #4ade80; border: 1px solid rgba(74, 222, 128, 0.4); }}
        .badge-cpp {{ background-color: rgba(96, 165, 250, 0.2); color: #60a5fa; border: 1px solid rgba(96, 165, 250, 0.4); }}
        .badge-opt {{ background-color: rgba(250, 204, 21, 0.2); color: #facc15; }}
        .badge-opt-O0 {{ background-color: rgba(59, 130, 246, 0.2); color: #60a5fa; border: 1px solid rgba(59, 130, 246, 0.4); }}
        .badge-opt-O1 {{ background-color: rgba(6, 182, 212, 0.2); color: #22d3ee; border: 1px solid rgba(6, 182, 212, 0.4); }}
        .badge-opt-O2 {{ background-color: rgba(34, 197, 94, 0.2); color: #4ade80; border: 1px solid rgba(34, 197, 94, 0.4); }}
        .badge-opt-O3 {{ background-color: rgba(234, 179, 8, 0.2); color: #facc15; border: 1px solid rgba(234, 179, 8, 0.4); }}
        .badge-opt-Os {{ background-color: rgba(168, 85, 247, 0.2); color: #c084fc; border: 1px solid rgba(168, 85, 247, 0.4); }}
        .bar-container {{
            background-color: var(--bg-main);
            height: 20px;
            border-radius: 4px;
            overflow: hidden;
            position: relative;
            margin-bottom: 6px;
        }}
        .bar-fill {{
            height: 100%;
            border-radius: 4px;
            transition: width 0.3s ease;
        }}
        .btn {{
            background-color: var(--primary);
            color: #0f172a;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            transition: opacity 0.2s;
        }}
        .btn:hover {{
            opacity: 0.9;
        }}
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div>
                <h1>⚡ TCC / GCC / Clang Benchmark & RAM Report</h1>
                <div class="subtitle">Performance, Memory Consumption (Peak RSS), and Binary Size Analysis across C/C++ Optimization Flags and Architectures</div>
            </div>
            <div>
                <button class="btn" onclick="exportJSON()">📥 Export JSON Data</button>
            </div>
        </header>

        <!-- KPI Summary Cards -->
        <div class="stats-grid" id="statsGrid">
            <div class="stat-card">
                <div class="label">TCC / T++ Compile Speedup</div>
                <div class="value" id="kpiCompileSpeedup">--x</div>
                <div class="desc">Faster compilation than GCC / Clang</div>
            </div>
            <div class="stat-card">
                <div class="label">TCC Compiler RAM Reduction</div>
                <div class="value" id="kpiRamReduction">--x</div>
                <div class="desc">Less compiler memory (Peak RSS)</div>
            </div>
            <div class="stat-card">
                <div class="label">Runtime Peak RAM</div>
                <div class="value" id="kpiExecRam">-- KB</div>
                <div class="desc">Average execution memory (Max RSS)</div>
            </div>
            <div class="stat-card">
                <div class="label">Average Binary Size</div>
                <div class="value" id="kpiBinSize">-- KB</div>
                <div class="desc">Compact native executables</div>
            </div>
            <div class="stat-card">
                <div class="label">Benchmark Test Runs</div>
                <div class="value" id="kpiTotalRuns">0</div>
                <div class="desc">Across all optimization levels (-O0 to -Os)</div>
            </div>
        </div>

        <!-- Filter Controls -->
        <div class="controls-card">
            <div class="control-group">
                <label for="filterLang">Language:</label>
                <select id="filterLang" onchange="applyFilters()">
                    <option value="ALL" selected>All (C & C++)</option>
                    <option value="c">C</option>
                    <option value="cpp">C++</option>
                </select>
            </div>
            <div class="control-group">
                <label for="filterArch">Architecture:</label>
                <select id="filterArch" onchange="applyFilters()">
                    <option value="ALL" selected>All Architectures</option>
                </select>
            </div>
            <div class="control-group">
                <label for="filterOpt">Optimization:</label>
                <select id="filterOpt" onchange="applyFilters()">
                    <option value="ALL" selected>All Levels (-O0, -O1, -O2, -O3, -Os)</option>
                    <option value="-O0">-O0</option>
                    <option value="-O1">-O1</option>
                    <option value="-O2">-O2</option>
                    <option value="-O3">-O3</option>
                    <option value="-Os">-Os</option>
                </select>
            </div>
            <div class="control-group">
                <label for="filterBench">Benchmark:</label>
                <select id="filterBench" onchange="applyFilters()">
                    <option value="ALL" selected>All Tasks</option>
                </select>
            </div>
            <div class="control-group" style="margin-left: auto;">
                <input type="text" id="searchBox" placeholder="Search results..." oninput="applyFilters()">
            </div>
        </div>

        <!-- Visual Comparison Charts -->
        <div class="charts-section">
            <div class="chart-card">
                <h3>⚡ Compilation Time (ms) — Lower is Better</h3>
                <div id="chartCompileTime"></div>
            </div>
            <div class="chart-card">
                <h3>🧠 Compiler Peak RAM (Max RSS KB) — Lower is Better</h3>
                <div id="chartCompilerRam"></div>
            </div>
            <div class="chart-card">
                <h3>🚀 Runtime Execution Time (ms) — Lower is Better</h3>
                <div id="chartExecTime"></div>
            </div>
            <div class="chart-card">
                <h3>💾 Runtime Execution Peak RAM (Max RSS KB) — Lower is Better</h3>
                <div id="chartExecRam"></div>
            </div>
            <div class="chart-card">
                <h3>📦 Executable Binary Size (KB) — Lower is Better</h3>
                <div id="chartBinSize"></div>
            </div>
        </div>

        <!-- Detailed Results Table -->
        <div class="table-card">
            <div class="table-header">
                <h3>📊 Detailed Benchmark Records</h3>
                <span id="recordCount" style="font-size: 13px; color: var(--text-muted);">Showing 0 records</span>
            </div>
            <div class="table-responsive">
                <table id="dataTable">
                    <thead>
                        <tr>
                            <th onclick="sortTable('benchmark')">Task ⬍</th>
                            <th onclick="sortTable('lang')">Lang ⬍</th>
                            <th onclick="sortTable('arch')">Arch ⬍</th>
                            <th onclick="sortTable('compiler')">Compiler ⬍</th>
                            <th onclick="sortTable('opt')">Opt ⬍</th>
                            <th onclick="sortTable('compile_time_ms')">Compile Time (ms) ⬍</th>
                            <th onclick="sortTable('compiler_peak_rss_kb')">Compiler RAM (KB) ⬍</th>
                            <th onclick="sortTable('exec_time_ms')">Run Time (ms) ⬍</th>
                            <th onclick="sortTable('exec_peak_rss_kb')">Run RAM (KB) ⬍</th>
                            <th onclick="sortTable('binary_size_bytes')">Binary Size ⬍</th>
                        </tr>
                    </thead>
                    <tbody id="tableBody">
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        const rawData = {results_json_str};
        let currentSort = {{ key: 'benchmark', asc: true }};

        function init() {{
            // Populate Architectures dropdown
            const archs = Array.from(new Set(rawData.map(d => d.arch))).sort();
            const archSelect = document.getElementById('filterArch');
            archSelect.innerHTML = '<option value="ALL" selected>All Architectures</option>';
            archs.forEach(a => {{
                const opt = document.createElement('option');
                opt.value = a;
                opt.textContent = a;
                archSelect.appendChild(opt);
            }});

            // Populate Optimizations dropdown
            const opts = Array.from(new Set(rawData.map(d => d.opt))).sort();
            const optSelect = document.getElementById('filterOpt');
            optSelect.innerHTML = '<option value="ALL" selected>All Levels (-O0, -O1, -O2, -O3, -Os)</option>';
            ['-O0', '-O1', '-O2', '-O3', '-Os'].forEach(o => {{
                if (opts.includes(o)) {{
                    const optEl = document.createElement('option');
                    optEl.value = o;
                    optEl.textContent = o;
                    optSelect.appendChild(optEl);
                }}
            }});

            // Populate Benchmarks dropdown
            const benches = Array.from(new Set(rawData.map(d => d.benchmark))).sort();
            const benchSelect = document.getElementById('filterBench');
            benchSelect.innerHTML = '<option value="ALL" selected>All Tasks</option>';
            benches.forEach(b => {{
                const opt = document.createElement('option');
                opt.value = b;
                opt.textContent = b;
                benchSelect.appendChild(opt);
            }});

            applyFilters();
        }}

        function getFilteredData() {{
            const lang = document.getElementById('filterLang').value;
            const arch = document.getElementById('filterArch').value;
            const opt = document.getElementById('filterOpt').value;
            const bench = document.getElementById('filterBench').value;
            const search = document.getElementById('searchBox').value.toLowerCase();

            return rawData.filter(d => {{
                if (lang !== 'ALL' && d.lang !== lang) return false;
                if (arch !== 'ALL' && d.arch !== arch) return false;
                if (opt !== 'ALL' && d.opt !== opt) return false;
                if (bench !== 'ALL' && d.benchmark !== bench) return false;
                if (search) {{
                    const fullText = (d.benchmark + ' ' + d.compiler + ' ' + d.opt + ' ' + d.arch).toLowerCase();
                    if (!fullText.includes(search)) return false;
                }}
                return true;
            }});
        }}

        function applyFilters() {{
            const filtered = getFilteredData();
            updateKPIs(filtered);
            renderCharts(filtered);
            renderTable(filtered);
        }}

        function updateKPIs(data) {{
            document.getElementById('kpiTotalRuns').textContent = data.length;

            const tccRows = data.filter(d => (d.compiler.startsWith('tcc') || d.compiler === 't++') && d.compile_success);
            const gccClangRows = data.filter(d => (!d.compiler.startsWith('tcc') && d.compiler !== 't++') && d.compile_success);

            if (tccRows.length > 0 && gccClangRows.length > 0) {{
                const avgTccCompile = tccRows.reduce((a, b) => a + b.compile_time_ms, 0) / tccRows.length;
                const avgGccCompile = gccClangRows.reduce((a, b) => a + b.compile_time_ms, 0) / gccClangRows.length;
                const speedup = avgTccCompile > 0 ? (avgGccCompile / avgTccCompile).toFixed(1) : '1.0';
                document.getElementById('kpiCompileSpeedup').textContent = speedup + 'x';

                const avgTccRam = tccRows.reduce((a, b) => a + b.compiler_peak_rss_kb, 0) / tccRows.length;
                const avgGccRam = gccClangRows.reduce((a, b) => a + b.compiler_peak_rss_kb, 0) / gccClangRows.length;
                const ramRatio = avgTccRam > 0 ? (avgGccRam / avgTccRam).toFixed(1) : '1.0';
                document.getElementById('kpiRamReduction').textContent = ramRatio + 'x';
            }} else {{
                document.getElementById('kpiCompileSpeedup').textContent = '--';
                document.getElementById('kpiRamReduction').textContent = '--';
            }}

            if (data.length > 0) {{
                const avgBin = data.reduce((a, b) => a + b.binary_size_bytes, 0) / data.length / 1024;
                document.getElementById('kpiBinSize').textContent = avgBin.toFixed(1) + ' KB';

                const validExec = data.filter(d => d.exec_peak_rss_kb > 0);
                if (validExec.length > 0) {{
                    const avgExecRam = validExec.reduce((a, b) => a + b.exec_peak_rss_kb, 0) / validExec.length;
                    document.getElementById('kpiExecRam').textContent = Math.round(avgExecRam).toLocaleString() + ' KB';
                }} else {{
                    document.getElementById('kpiExecRam').textContent = '-- KB';
                }}
            }}
        }}

        function getCompilerColor(compiler) {{
            if (compiler.startsWith('tcc') || compiler === 't++') return '#38bdf8';
            if (compiler.startsWith('gcc') || compiler.startsWith('g++')) return '#f87171';
            return '#c084fc'; // clang
        }}

        function renderBarChart(containerId, data, metricKey, unit) {{
            const container = document.getElementById(containerId);
            container.innerHTML = '';

            // Aggregate by compiler
            const compMap = {{}};
            data.forEach(d => {{
                if (d[metricKey] > 0) {{
                    if (!compMap[d.compiler]) compMap[d.compiler] = {{ sum: 0, count: 0 }};
                    compMap[d.compiler].sum += d[metricKey];
                    compMap[d.compiler].count++;
                }}
            }});

            const items = Object.keys(compMap).map(c => ({{
                compiler: c,
                avg: compMap[c].sum / compMap[c].count
            }}));

            if (items.length === 0) {{
                container.innerHTML = '<div style="color: var(--text-muted); padding: 10px;">No metric data available for selection</div>';
                return;
            }}

            const maxVal = Math.max(...items.map(i => i.avg));

            items.sort((a, b) => a.avg - b.avg);

            items.forEach(item => {{
                const pct = maxVal > 0 ? (item.avg / maxVal * 100) : 0;
                const color = getCompilerColor(item.compiler);
                const row = document.createElement('div');
                row.style.marginBottom = '12px';
                row.innerHTML = `
                    <div style="display: flex; justify-content: space-between; font-size: 12px; margin-bottom: 4px;">
                        <span style="font-weight: 600;">${{item.compiler}}</span>
                        <span style="color: var(--text-muted);">${{item.avg.toFixed(1)}} ${{unit}}</span>
                    </div>
                    <div class="bar-container">
                        <div class="bar-fill" style="width: ${{Math.max(pct, 2)}}%; background-color: ${{color}};"></div>
                    </div>
                `;
                container.appendChild(row);
            }});
        }}

        function renderCharts(data) {{
            renderBarChart('chartCompileTime', data, 'compile_time_ms', 'ms');
            renderBarChart('chartCompilerRam', data, 'compiler_peak_rss_kb', 'KB');
            renderBarChart('chartExecTime', data, 'exec_time_ms', 'ms');
            renderBarChart('chartExecRam', data, 'exec_peak_rss_kb', 'KB');
            renderBarChart('chartBinSize', data.map(d => ({{...d, bin_kb: d.binary_size_bytes/1024}})), 'bin_kb', 'KB');
        }}

        function renderTable(data) {{
            const tbody = document.getElementById('tableBody');
            tbody.innerHTML = '';
            document.getElementById('recordCount').textContent = `Showing ${{data.length}} records`;

            data.sort((a, b) => {{
                let valA = a[currentSort.key];
                let valB = b[currentSort.key];
                if (typeof valA === 'string') {{
                    return currentSort.asc ? valA.localeCompare(valB) : valB.localeCompare(valA);
                }}
                return currentSort.asc ? valA - valB : valB - valA;
            }});

            data.forEach(d => {{
                const tr = document.createElement('tr');
                const compBadgeClass = (d.compiler.startsWith('tcc') || d.compiler === 't++') ? 'badge-tcc' : (d.compiler.startsWith('gcc') || d.compiler.startsWith('g++') ? 'badge-gcc' : 'badge-clang');
                const langBadgeClass = d.lang === 'c' ? 'badge-c' : 'badge-cpp';
                const optBadgeClass = 'badge-opt-' + d.opt.replace('-', '');

                tr.innerHTML = `
                    <td><strong>${{d.benchmark}}</strong></td>
                    <td><span class="badge ${{langBadgeClass}}">${{d.lang.toUpperCase()}}</span></td>
                    <td><span class="badge" style="background: rgba(255,255,255,0.08); color: #cbd5e1;">${{d.arch}}</span></td>
                    <td><span class="badge ${{compBadgeClass}}">${{d.compiler}}</span></td>
                    <td><span class="badge ${{optBadgeClass}}">${{d.opt}}</span></td>
                    <td style="color: var(--accent-green); font-weight: 600;">${{d.compile_time_ms.toFixed(1)}} ms</td>
                    <td>${{d.compiler_peak_rss_kb.toLocaleString()}} KB</td>
                    <td style="font-weight: 600;">${{d.exec_time_ms > 0 ? d.exec_time_ms.toFixed(1) + ' ms' : '<span style="color: var(--text-muted);">N/A</span>'}}</td>
                    <td>${{d.exec_peak_rss_kb > 0 ? d.exec_peak_rss_kb.toLocaleString() + ' KB' : '<span style="color: var(--text-muted);">N/A</span>'}}</td>
                    <td>${{(d.binary_size_bytes / 1024).toFixed(1)}} KB</td>
                `;
                tbody.appendChild(tr);
            }});
        }}

        function sortTable(key) {{
            if (currentSort.key === key) {{
                currentSort.asc = !currentSort.asc;
            }} else {{
                currentSort.key = key;
                currentSort.asc = true;
            }}
            applyFilters();
        }}

        function exportJSON() {{
            const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(rawData, null, 2));
            const downloadAnchor = document.createElement('a');
            downloadAnchor.setAttribute("href", dataStr);
            downloadAnchor.setAttribute("download", "compiler_benchmarks.json");
            document.body.appendChild(downloadAnchor);
            downloadAnchor.click();
            downloadAnchor.remove();
        }}

        window.onload = init;
    </script>
</body>
</html>
"""
    with open(report_file, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"\n[+] HTML Benchmark Report generated at: {report_file}")

def main():
    parser = argparse.ArgumentParser(description="Compiler Performance & RAM Benchmark Suite")
    parser.add_argument("--build-dir", default="build", help="Path to build directory")
    parser.add_argument("--src-dir", default=".", help="Path to source root")
    parser.add_argument("--tcc", default=None, help="Path to tcc executable")
    parser.add_argument("--html-out", default=None, help="Output HTML file path")
    parser.add_argument("--json-out", default=None, help="Output JSON file path")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4, help="Number of parallel worker threads (default: CPU cores)")
    parser.add_argument("-r", "--runs", "--exec-runs", dest="exec_runs", type=int, default=5, help="Number of runtime execution runs to average (default: 5)")
    parser.add_argument("--compile-runs", type=int, default=3, help="Number of compilation runs to average (default: 3)")
    parser.add_argument("--opt-levels", default="-O0,-O1,-O2,-O3,-Os", help="Comma-separated optimization levels")
    parser.add_argument("--archs", default="x86_64", help="Comma-separated target architectures (e.g. x86_64,i386,arm,arm64,riscv64)")
    parser.add_argument("--quick", action="store_true", help="Quick mode (only -O0 and -O2 on native arch with fewer iterations)")
    args = parser.parse_args()

    build_dir = os.path.abspath(args.build_dir)
    src_dir = os.path.abspath(args.src_dir)
    bench_dir = os.path.join(src_dir, "tests", "benchmarks")
    
    html_out = args.html_out or os.path.join(build_dir, "benchmark_report.html")
    json_out = args.json_out or os.path.join(build_dir, "benchmark_report.json")

    opt_levels = ["-O0", "-O2"] if args.quick else args.opt_levels.split(",")
    archs = ["x86_64"] if args.quick else args.archs.split(",")
    jobs = max(1, args.jobs)
    exec_runs = 2 if args.quick else max(1, args.exec_runs)
    compile_runs = 1 if args.quick else max(1, args.compile_runs)

    compilers = discover_compilers(build_dir, args.tcc)
    emulators = discover_emulators()

    print("=" * 60)
    print("🚀 Compiler Performance & RAM Benchmark Suite")
    print(f"   Compilers: {list(compilers.keys())}")
    print(f"   Emulators: {list(emulators.keys())}")
    print(f"   Optimization Levels: {opt_levels}")
    print(f"   Target Architectures: {archs}")
    print(f"   Parallel Workers: {jobs}")
    print(f"   Averaging Iterations: {exec_runs} execution runs, {compile_runs} compile runs")
    print("=" * 60)

    c_benchmarks = sorted(glob.glob(os.path.join(bench_dir, "c", "*.c")))
    cpp_benchmarks = sorted(glob.glob(os.path.join(bench_dir, "cpp", "*.cpp")))

    # Collect tasks
    tasks = []
    
    # 1. C benchmarks
    for bench in c_benchmarks:
        for arch in archs:
            for opt in opt_levels:
                comp_list = []
                if "gcc" in compilers: comp_list.append(("gcc", compilers["gcc"]))
                if "clang" in compilers: comp_list.append(("clang", compilers["clang"]))
                if arch == "x86_64" and "tcc" in compilers:
                    comp_list.append(("tcc", compilers["tcc"]))
                elif f"tcc-{arch}" in compilers:
                    comp_list.append((f"tcc-{arch}", compilers[f"tcc-{arch}"]))

                for comp_name, comp_bin in comp_list:
                    tasks.append((bench, "c", comp_name, comp_bin, opt, arch, build_dir, emulators, compile_runs, exec_runs))

    # 2. C++ benchmarks
    for bench in cpp_benchmarks:
        for arch in archs:
            for opt in opt_levels:
                comp_list = []
                if "g++" in compilers: comp_list.append(("g++", compilers["g++"]))
                if "clang++" in compilers: comp_list.append(("clang++", compilers["clang++"]))
                if arch == "x86_64" and "t++" in compilers:
                    comp_list.append(("t++", compilers["t++"]))
                elif f"tcc-{arch}" in compilers:
                    comp_list.append((f"t++-{arch}", compilers[f"tcc-{arch}"]))

                for comp_name, comp_bin in comp_list:
                    tasks.append((bench, "cpp", comp_name, comp_bin, opt, arch, build_dir, emulators, compile_runs, exec_runs))

    print(f"\n[*] Executing {len(tasks)} benchmark configurations across {jobs} worker threads (each averaged over {exec_runs} runs)...\n")
    start_time = time.perf_counter()

    lock = threading.Lock()
    completed_count = 0
    total_count = len(tasks)

    def worker(task):
        nonlocal completed_count
        bench, lang, comp_name, comp_bin, opt, arch, bdir, emus, c_runs, e_runs = task
        res = build_and_run_benchmark(bench, lang, comp_name, comp_bin, opt, arch, bdir, emus, compile_runs=c_runs, exec_runs=e_runs)
        with lock:
            completed_count += 1
            status = "OK" if res["compile_success"] else "FAIL"
            print(f"  [{completed_count:>3}/{total_count}] [{status}] {res['benchmark']:<18} | {comp_name:<8} {opt:<4} ({arch}): Compile {res['compile_time_ms']:>5.1f}ms (RAM {res['compiler_peak_rss_kb']:>5}KB) | Run {res['exec_time_ms']:>5.1f}ms (RAM {res['exec_peak_rss_kb']:>5}KB) | Bin {res['binary_size_bytes']:>5}B [avg of {e_runs} runs]")
        return res

    results = []
    if jobs == 1:
        for t in tasks:
            results.append(worker(t))
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as executor:
            results = list(executor.map(worker, tasks))

    elapsed_total = time.perf_counter() - start_time
    print(f"\n[✓] Completed {len(results)} benchmark runs in {elapsed_total:.2f} seconds ({len(results)/elapsed_total:.1f} runs/sec).")

    # Save JSON and HTML
    with open(json_out, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)
    print(f"\n[+] JSON Results saved to: {json_out}")

    generate_html_report(results, html_out)
    return 0

if __name__ == "__main__":
    sys.exit(main())
