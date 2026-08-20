#!/usr/bin/env python3
import sys
import os
import subprocess
import argparse
import glob
import re

def run_cmd(cmd, cwd=None, input_text=None):
    res = subprocess.run(
        cmd,
        cwd=cwd,
        input=input_text,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )
    return res.returncode, res.stdout

def run_tests2(tcc, host_cc, src_dir, build_dir, single_test=None):
    tests2_dir = os.path.join(src_dir, "tests", "tests2")
    c_files = sorted(glob.glob(os.path.join(tests2_dir, "[0-9][0-9]_*.c")) + 
                     glob.glob(os.path.join(tests2_dir, "[0-9][0-9][0-9]_*.c")))
    if single_test:
        c_files = [f for f in c_files if os.path.basename(f).startswith(single_test)]
    
    tcc_flags = ["-B" + build_dir, "-I" + os.path.join(src_dir, "include"), "-I" + src_dir, "-I" + build_dir]
    failed = []
    
    # Skips based on target architecture / OS (default x86_64 Linux)
    skip = {
        "34_array_assignment.c",
        "98_al_ax_extend.c",
        "99_fastcall.c",
        "138_arm64_encoding.c",
        "139_arm64_errors.c",
        "140_arm64_extasm.c",
        "141_riscv_asm.c",
        "145_winarm64_interlocked.c",
    }
    
    for c_file in c_files:
        base = os.path.basename(c_file)
        test_name = os.path.splitext(base)[0]
        if base in skip:
            continue
        expect_file = os.path.splitext(c_file)[0] + ".expect"
        if not os.path.exists(expect_file):
            continue
        with open(expect_file, "r", errors="ignore") as ef:
            expect_text = ef.read()

        args = []
        extra_flags = []
        norun = False
        
        if test_name == "31_args":
            args = ["arg1", "arg2", "arg3", "arg4", "arg5"]
        elif test_name == "46_grep":
            args = ['[^* ]*[:a:d: ]+\\:\\*-/: $', c_file]
        elif test_name == "76_dollars_in_identifiers":
            extra_flags = ["-fdollars-in-identifiers"]
        elif test_name in ("22_floating_point", "24_math_library"):
            extra_flags = ["-lm"]
        elif test_name in ("60_errors_and_warnings", "96_nodata_wanted", "125_atomic_misc", "128_run_atexit"):
            extra_flags = ["-dt"]
        elif test_name in ("106_versym", "124_atomic_counter", "144_tls"):
            extra_flags = ["-pthread"]
            if test_name in ("106_versym", "144_tls"):
                norun = True
        elif test_name == "108_constructor":
            norun = True
        elif test_name in ("114_bound_signal", "126_bound_global"):
            extra_flags = ["-b"]
            norun = True
        elif test_name in ("115_bound_setjmp", "116_bound_setjmp2", "121_struct_return", "122_vla_reuse", "132_bound_test"):
            extra_flags = ["-b"]
        elif test_name == "112_backtrace":
            extra_flags = ["-dt", "-b"]
        
        out = ""
        if test_name == "104_inline":
            plus_file = os.path.join(tests2_dir, "104+_inline.c")
            cmd = [tcc] + tcc_flags + extra_flags + [plus_file, "-run", c_file] + args
            rc, out = run_cmd(cmd)
        elif test_name == "120_alias":
            plus_file = os.path.join(tests2_dir, "120+_alias.c")
            exe_file = os.path.join(build_dir, "120_alias.exe")
            cmd_c = [tcc] + tcc_flags + extra_flags + ["-o", exe_file, c_file, plus_file]
            rc, out = run_cmd(cmd_c)
            if rc != 0:
                print(f"Test {test_name} compile FAILED:\n{out}")
                failed.append(test_name)
                continue
            rc, out = run_cmd([exe_file])
            if os.path.exists(exe_file):
                os.remove(exe_file)
        elif test_name == "113_btdll":
            a1_so = os.path.join(build_dir, "a1.so")
            a2_so = os.path.join(build_dir, "a2.so")
            exe_file = os.path.join(build_dir, "113_btdll.exe")
            run_cmd([tcc] + tcc_flags + ["-bt", c_file, "-shared", "-D", "DLL=1", "-o", a1_so])
            run_cmd([tcc] + tcc_flags + ["-bt", c_file, "-shared", "-D", "DLL=2", "-o", a2_so])
            run_cmd([tcc] + tcc_flags + ["-bt", c_file, a1_so, a2_so, f"-Wl,-rpath={build_dir}", "-o", exe_file])
            rc, out = run_cmd([exe_file])
            for f in (a1_so, a2_so, exe_file):
                if os.path.exists(f):
                    os.remove(f)
        elif test_name == "117_builtins":
            rc1, out1 = run_cmd([tcc] + tcc_flags + ["-run", c_file])
            rc2, out2 = run_cmd([tcc] + tcc_flags + ["-b", "-run", c_file])
            out = out1 + out2
        elif test_name == "146_tls_extern":
            o1 = os.path.join(build_dir, "146_tls_extern-main.o")
            o2 = os.path.join(build_dir, "146_tls_extern-defs.o")
            exe_file = os.path.join(build_dir, "146_tls_extern.exe")
            run_cmd([tcc] + tcc_flags + ["-c", c_file, "-o", o1])
            run_cmd([tcc] + tcc_flags + ["-c", c_file, "-DDEFS", "-o", o2])
            run_cmd([host_cc, "-no-pie", "-Wl,-z,noexecstack", o1, o2, "-o", exe_file])
            rc, out = run_cmd([exe_file])
            for f in (o1, o2, exe_file):
                if os.path.exists(f):
                    os.remove(f)
        elif test_name == "148_linker_symbols":
            full_out = []
            for T in range(1, 7):
                full_out.append(f"\n--- TEST {T} ---")
                o_f = os.path.join(build_dir, f"148-{T}.o")
                so_f = os.path.join(build_dir, f"148-{T}.so")
                exe_f = os.path.join(build_dir, f"148-{T}.exe")
                run_cmd([tcc] + tcc_flags + [c_file, f"-DTEST={T}+100", "-c", "-fcommon", "-o", o_f])
                run_cmd([tcc] + tcc_flags + [c_file, f"-DTEST={T}+200", "-shared", "-o", so_f, "-fPIC"])
                run_cmd([tcc] + tcc_flags + [c_file, f"-DTEST={T}", f"-Wl,-rpath={build_dir}", "-o", exe_f, o_f, so_f, "-no-pie"])
                rc, res_o = run_cmd([exe_f])
                full_out.append(res_o.strip())
                for f in (o_f, so_f, exe_f):
                    if os.path.exists(f):
                        os.remove(f)
            out = "\n".join(full_out) + "\n"
        elif norun:
            exe_file = os.path.join(build_dir, f"{test_name}.exe")
            cmd_c = [tcc] + tcc_flags + extra_flags + ["-o", exe_file, c_file]
            rc, out = run_cmd(cmd_c)
            if rc != 0:
                print(f"Test {test_name} compile FAILED:\n{out}")
                failed.append(test_name)
                continue
            rc, out = run_cmd([exe_file] + args)
            if os.path.exists(exe_file):
                os.remove(exe_file)
        else:
            cmd = [tcc] + tcc_flags + extra_flags + ["-run", c_file] + args
            rc, out = run_cmd(cmd)

        out_clean = out.replace(tests2_dir + "/", "").replace(tests2_dir + "\\", "")
        out_clean = out_clean.replace("\r\n", "\n")
        expect_clean = expect_text.replace("\r\n", "\n")
        
        if test_name in ("112_backtrace", "113_btdll", "126_bound_global"):
            out_clean = re.sub(r'[0-9A-Fa-fx]{5,}', '........', out_clean)
            out_clean = re.sub(r'0x[0-9A-Fa-f]+', '0x?', out_clean)
            expect_clean = re.sub(r'[0-9A-Fa-fx]{5,}', '........', expect_clean)
            expect_clean = re.sub(r'0x[0-9A-Fa-f]+', '0x?', expect_clean)

        # Whitespace-insensitive comparison (diff -b style)
        out_lines = [" ".join(l.split()) for l in out_clean.strip().splitlines() if l.strip()]
        expect_lines = [" ".join(l.split()) for l in expect_clean.strip().splitlines() if l.strip()]

        if out_lines != expect_lines:
            print(f"Test {test_name} FAILED!")
            print(f"Expected:\n{expect_clean[:500]}")
            print(f"Got:\n{out_clean[:500]}")
            failed.append(test_name)
        else:
            print(f"Test: {test_name}... OK")

    if failed:
        print(f"\n{len(failed)} tests failed: {failed}")
        return 1
    print("\nAll tests2 passed!")
    return 0

def run_tests_pp(tcc, src_dir, build_dir):
    pp_dir = os.path.join(src_dir, "tests", "pp")
    files = sorted(glob.glob(os.path.join(pp_dir, "*.[cS]")))
    tcc_flags = ["-B" + build_dir, "-I" + os.path.join(src_dir, "include"), "-I" + src_dir, "-I" + build_dir]
    failed = []
    
    for f in files:
        base = os.path.basename(f)
        test_name = os.path.splitext(base)[0]
        expect_file = os.path.splitext(f)[0] + ".expect"
        if not os.path.exists(expect_file):
            continue
        with open(expect_file, "r", errors="ignore") as ef:
            expect_text = ef.read()

        cmd = [tcc] + tcc_flags + ["-E", "-P", f]
        rc, out = run_cmd(cmd)
        
        out_clean = out.replace(pp_dir + "/", "").replace(pp_dir + "\\", "")
        out_clean = out_clean.replace("\r\n", "\n").strip()
        expect_clean = expect_text.replace("\r\n", "\n").strip()
        
        # Whitespace-tolerant comparison (diff -w for 02, diff -b for others)
        if test_name == "02":
            differs = ("".join(out_clean.split()) != "".join(expect_clean.split()))
        else:
            out_lines = [" ".join(l.split()) for l in out_clean.splitlines() if l.strip()]
            expect_lines = [" ".join(l.split()) for l in expect_clean.strip().splitlines() if l.strip()]
            differs = (out_lines != expect_lines)

        if differs:
            print(f"PPTest {test_name} FAILED!")
            failed.append(test_name)
        else:
            print(f"PPTest {test_name}... OK")

    if failed:
        print(f"\n{len(failed)} pp tests failed: {failed}")
        return 1
    print("\nAll pp tests passed!")
    return 0

def run_btest(tcc, src_dir, build_dir):
    boundtest_c = os.path.join(src_dir, "tests", "boundtest.c")
    tcc_flags = ["-B" + build_dir, "-I" + os.path.join(src_dir, "include"), "-I" + src_dir, "-I" + build_dir]
    bounds_ok = [1, 4, 8, 10, 14, 16]
    bounds_fail = [2, 5, 6, 7, 9, 11, 12, 13, 15, 17, 18]
    
    for i in bounds_ok:
        cmd = [tcc] + tcc_flags + ["-b", "-run", boundtest_c, str(i)]
        rc, out = run_cmd(cmd)
        if rc != 0:
            print(f"Failed positive test {i}")
            return 1
        print(f"Test {i} succeeded as expected")
        
    for i in bounds_fail:
        cmd = [tcc] + tcc_flags + ["-b", "-bt1", "-run", boundtest_c, str(i)]
        rc, out = run_cmd(cmd)
        if rc == 0:
            print(f"Failed negative test {i} (expected failure)")
            return 1
        print(f"Test {i} failed as expected")
        
    print("Bound-Test OK")
    return 0

def run_tcctest(tcc, host_cc, src_dir, build_dir):
    tcctest_c = os.path.join(src_dir, "tests", "tcctest.c")
    tcc_flags = ["-B" + build_dir, "-I" + os.path.join(src_dir, "include"), "-I" + src_dir, "-I" + build_dir]
    
    ref_exe = os.path.join(build_dir, "tcctest.gcc.exe")
    rc, out = run_cmd([host_cc, "-o", ref_exe, tcctest_c, "-I" + src_dir, "-I" + build_dir, "-w", "-O0", "-std=gnu99", "-fno-omit-frame-pointer", "-lm"])
    if rc != 0:
        print(f"Failed to compile tcctest with host compiler: {out}")
        return 1
    rc, ref_out = run_cmd([ref_exe])
    if os.path.exists(ref_exe):
        os.remove(ref_exe)
        
    cmd = [tcc] + tcc_flags + ["-w", "-run", tcctest_c]
    rc, tcc_out = run_cmd(cmd)
    
    if ref_out.strip() != tcc_out.strip():
        print("tcctest output mismatch!")
        return 1
        
    print("Auto Test OK")
    return 0

def run_cross_test(src_dir, build_dir):
    tcctest_c = os.path.join(src_dir, "tests", "tcctest.c")
    ex3_c = os.path.join(src_dir, "examples", "ex3.c")
    cross_compilers = [
        "i386-tcc", "i386-win32-tcc", "x86_64-tcc", "x86_64-win32-tcc", "x86_64-osx-tcc",
        "arm-tcc", "arm64-tcc", "arm64-win32-tcc", "arm64-osx-tcc", "riscv64-tcc", "c67-tcc"
    ]
    for cc in cross_compilers:
        exe = os.path.join(build_dir, cc)
        if not os.path.exists(exe):
            continue
        test_file = ex3_c if "c67" in cc else tcctest_c
        obj = os.path.join(build_dir, f"test_{cc}.o")
        flags = ["-I" + build_dir, "-I" + src_dir, "-I" + os.path.join(src_dir, "include")]
        rc, out = run_cmd([exe, "-c", test_file, "-o", obj] + flags)
        if rc != 0:
            print(f"Cross test {cc} on {test_file} failed: {out}")
            return 1
        if os.path.exists(obj):
            os.remove(obj)
        print(f"  . {cc} OK")
    return 0

def run_tests_cpp(tcc, src_dir, build_dir, single_test=None):
    tests_cpp_dir = os.path.join(src_dir, "tests", "tests_cpp")
    cpp_files = sorted(glob.glob(os.path.join(tests_cpp_dir, "[0-9][0-9]_*.cpp")))
    if single_test:
        cpp_files = [f for f in cpp_files if os.path.basename(f).startswith(single_test)]
    
    tcc_flags = ["-B" + build_dir, "-I" + os.path.join(src_dir, "include"), "-I" + src_dir, "-I" + build_dir]
    failed = []
    
    for cpp_file in cpp_files:
        base = os.path.basename(cpp_file)
        test_name = os.path.splitext(base)[0]
        expect_file = os.path.splitext(cpp_file)[0] + ".expect"
        if not os.path.exists(expect_file):
            continue
        with open(expect_file, "r", errors="ignore") as ef:
            expect_text = ef.read()

        cmd = [tcc] + tcc_flags + ["-run", cpp_file]
        rc, out = run_cmd(cmd)
        if rc != 0:
            print(f"Test {test_name} FAILED (rc={rc}):\n{out}")
            failed.append(test_name)
            continue
        if out != expect_text:
            print(f"Test {test_name} FAILED: output mismatch")
            print("EXPECTED:\n" + expect_text)
            print("GOT:\n" + out)
            failed.append(test_name)
            continue
        print(f"  . {test_name} OK")
    
    if failed:
        print(f"\n{len(failed)} C++ test(s) failed: {', '.join(failed)}")
        return 1
    print(f"\nAll {len(cpp_files)} C++ tests passed successfully!")
    return 0

def main():
    parser = argparse.ArgumentParser(description="Test runner for TinyCC")
    parser.add_argument("--test-type", required=True, choices=["tests2", "pp", "btest", "tcctest", "cross_test", "hello_exe", "hello_run", "vla_test", "asm_c_connect", "cpp"])
    parser.add_argument("--tcc", default="tcc")
    parser.add_argument("--host-cc", default="gcc")
    parser.add_argument("--src-dir", required=True)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--single-test", default=None)
    args = parser.parse_args()

    tcc_flags = ["-B" + args.build_dir, "-I" + os.path.join(args.src_dir, "include"), "-I" + args.src_dir, "-I" + args.build_dir]

    if args.test_type == "cpp":
        return run_tests_cpp(args.tcc, args.src_dir, args.build_dir, args.single_test)
    elif args.test_type == "tests2":
        return run_tests2(args.tcc, args.host_cc, args.src_dir, args.build_dir, args.single_test)
    elif args.test_type == "pp":
        return run_tests_pp(args.tcc, args.src_dir, args.build_dir)
    elif args.test_type == "btest":
        return run_btest(args.tcc, args.src_dir, args.build_dir)
    elif args.test_type == "tcctest":
        return run_tcctest(args.tcc, args.host_cc, args.src_dir, args.build_dir)
    elif args.test_type == "cross_test":
        return run_cross_test(args.src_dir, args.build_dir)
    elif args.test_type == "hello_exe":
        ex1_c = os.path.join(args.src_dir, "examples", "ex1.c")
        out_exe = os.path.join(args.build_dir, "hello_exe.tmp")
        rc, out = run_cmd([args.tcc] + tcc_flags + [ex1_c, "-o", out_exe])
        if rc != 0:
            print(f"hello_exe compilation failed:\n{out}")
            return 1
        rc, out = run_cmd([out_exe])
        if os.path.exists(out_exe):
            os.remove(out_exe)
        if rc != 0 or "Hello World" not in out:
            print(f"hello_exe run failed:\n{out}")
            return 1
        print("hello_exe OK")
        return 0
    elif args.test_type == "hello_run":
        ex1_c = os.path.join(args.src_dir, "examples", "ex1.c")
        rc, out = run_cmd([args.tcc] + tcc_flags + ["-run", ex1_c])
        if rc != 0 or "Hello World" not in out:
            print(f"hello_run failed:\n{out}")
            return 1
        print("hello_run OK")
        return 0
    elif args.test_type == "vla_test":
        vla_c = os.path.join(args.src_dir, "tests", "vla_test.c")
        rc, out = run_cmd([args.tcc] + tcc_flags + ["-run", vla_c])
        if rc != 0:
            print(f"vla_test failed:\n{out}")
            return 1
        print("vla_test OK")
        return 0
    elif args.test_type == "asm_c_connect":
        f1 = os.path.join(args.src_dir, "tests", "asm-c-connect-1.c")
        f2 = os.path.join(args.src_dir, "tests", "asm-c-connect-2.c")
        out_exe = os.path.join(args.build_dir, "asm-c-connect.tmp")
        rc, out = run_cmd([args.tcc] + tcc_flags + [f1, f2, "-o", out_exe])
        if rc != 0:
            print(f"asm_c_connect failed:\n{out}")
            return 1
        rc, out = run_cmd([out_exe])
        if os.path.exists(out_exe):
            os.remove(out_exe)
        if rc != 0:
            return 1
        print("asm_c_connect OK")
        return 0
    elif args.test_type == "benchmarks":
        import run_benchmarks
        bench_py = os.path.join(args.src_dir, "tests", "run_benchmarks.py")
        rc, out = run_cmd([sys.executable, bench_py, "--build-dir", args.build_dir, "--src-dir", args.src_dir, "--tcc", args.tcc, "--quick"])
        print(out)
        return rc

if __name__ == "__main__":
    sys.exit(main())
