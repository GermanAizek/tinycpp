#!/usr/bin/env python3
"""
Master Test Runner for the Complete 200-Test Compiler Verification & Benchmark Suite.
Categorized into:
  I.   Frontend: Lexer & Parser (1-30)
  II.  Type System & Semantics (31-65)
  III. Middle-end Optimizations (66-110)
  IV.  Code Generation & Backend (111-140)
  V.   Algorithmic & Complex Workloads (141-170)
  VI.  Compiler Scale & Robustness (171-200)
"""

import os
import sys
import glob
import subprocess
import time
import argparse

CATEGORY_NAMES = {
    "01_frontend": "I.   Frontend: Lexer & Parser (Tests 1-30)",
    "02_semantics": "II.  Type System & Semantics (Tests 31-65)",
    "03_middle_end": "III. Middle-end Optimizations (Tests 66-110)",
    "04_backend": "IV.  Code Generation & Backend (Tests 111-140)",
    "05_workloads": "V.   Algorithmic & Complex Workloads (Tests 141-170)",
    "06_stress": "VI.  Compiler Scale & Robustness (Tests 171-200)"
}

def run_test(tcc_bin, bdir, src_file, opt_level="-O2"):
    base = os.path.basename(src_file)
    test_name = os.path.splitext(base)[0]
    out_exe = os.path.join(bdir, f"suite200_{test_name}.exe")
    
    tcc_inc = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "include"))
    tcc_b = os.path.abspath(bdir)
    
    cmd_compile = [tcc_bin, "-B" + tcc_b, "-I" + tcc_inc, opt_level, src_file, "-o", out_exe, "-lm"]
    
    t0 = time.perf_counter()
    p_comp = subprocess.run(cmd_compile, capture_output=True, text=True)
    t1 = time.perf_counter()
    compile_ms = (t1 - t0) * 1000.0
    
    if p_comp.returncode != 0:
        if os.path.exists(out_exe):
            try: os.remove(out_exe)
            except Exception: pass
        return False, compile_ms, 0, f"Compile FAIL (rc={p_comp.returncode}): {p_comp.stderr.strip()}"
    
    t2 = time.perf_counter()
    p_run = subprocess.run([out_exe], capture_output=True, text=True)
    t3 = time.perf_counter()
    run_ms = (t3 - t2) * 1000.0
    
    if os.path.exists(out_exe):
        try: os.remove(out_exe)
        except Exception: pass
        
    if p_run.returncode != 0:
        return False, compile_ms, run_ms, f"Run CRASH (rc={p_run.returncode}): {p_run.stderr.strip()}"
        
    return True, compile_ms, run_ms, p_run.stdout.strip()

def main():
    parser = argparse.ArgumentParser(description="200-Test Compiler Suite Runner")
    parser.add_argument("--tcc", default="./tcc", help="Path to tcc binary")
    parser.add_argument("--build-dir", default="build", help="Build directory")
    parser.add_argument("--opt", default="-O2", help="Optimization flag (-O0..-O3, -Os)")
    parser.add_argument("--category", default=None, help="Filter category (e.g. 01_frontend, 05_workloads)")
    args = parser.parse_args()

    suite_dir = os.path.join(os.path.dirname(__file__), "test_suite_200")
    os.makedirs(args.build_dir, exist_ok=True)
    
    cats = sorted([d for d in os.listdir(suite_dir) if os.path.isdir(os.path.join(suite_dir, d))])
    if args.category:
        cats = [c for c in cats if args.category in c]

    total_passed = 0
    total_failed = 0
    
    print("=" * 80)
    print(f"🚀 Running 200-Test Compiler Verification Suite ({args.opt})")
    print(f"   Compiler: {args.tcc}")
    print("=" * 80)
    
    for cat in cats:
        cat_path = os.path.join(suite_dir, cat)
        files = sorted(glob.glob(os.path.join(cat_path, "*.c")) + glob.glob(os.path.join(cat_path, "*.cpp")))
        cat_title = CATEGORY_NAMES.get(cat, cat)
        print(f"\n📂 {cat_title} [{len(files)} tests]:")
        print("-" * 80)
        
        passed_cat = 0
        for f in files:
            t_name = os.path.splitext(os.path.basename(f))[0]
            ok, c_ms, r_ms, out = run_test(args.tcc, args.build_dir, f, args.opt)
            if ok:
                passed_cat += 1
                total_passed += 1
                print(f"  [✓ PASS] {t_name:<32} | Compile: {c_ms:5.1f}ms | Exec: {r_ms:5.1f}ms | Output: {out}")
            else:
                total_failed += 1
                print(f"  [✗ FAIL] {t_name:<32} | {out}")
        print(f"--> Category Result: {passed_cat}/{len(files)} passed.")

    print("\n" + "=" * 80)
    print(f"🏁 Final Result: {total_passed}/{total_passed + total_failed} tests passed.")
    print("=" * 80)
    return 0 if total_failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
