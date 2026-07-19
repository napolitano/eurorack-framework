#!/usr/bin/env python3
"""Build and run every native test without requiring PlatformIO."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--sanitizers", action="store_true")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    compiler = shutil.which(args.compiler)
    if compiler is None:
        print(f"Compiler not found: {args.compiler}", file=sys.stderr)
        return 2

    build = root / ".native-tests"
    if build.exists():
        shutil.rmtree(build)
    build.mkdir()

    flags = [
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wconversion",
        "-Wsign-conversion",
        "-Wshadow",
        "-Wundef",
        "-Wformat=2",
        "-Wnull-dereference",
        "-Wdouble-promotion",
        "-Werror",
        "-I",
        str(root / "include"),
        "-I",
        str(root / "tools/test_support"),
    ]

    if args.sanitizers:
        flags += [
            "-O1",
            "-g",
            "-fno-omit-frame-pointer",
            "-fsanitize=address,undefined",
        ]
    else:
        flags += ["-O2"]

    sources = sorted((root / "src").rglob("*.cpp"))
    tests = sorted((root / "tests/native").glob("test_*/test_main.cpp"))

    objects = []
    for index, source in enumerate(sources):
        object_path = build / f"source_{index}.o"
        result = subprocess.run(
            [compiler, *flags, "-c", str(source), "-o", str(object_path)],
            cwd=root,
            text=True,
            capture_output=True,
        )
        if result.returncode != 0:
            print(f"[SOURCE BUILD FAIL] {source}", file=sys.stderr)
            print(result.stdout, file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1
        objects.append(object_path)

    passed = 0
    for test in tests:
        executable = build / test.parent.name
        test_object = build / f"{test.parent.name}.o"

        result = subprocess.run(
            [compiler, *flags, "-c", str(test), "-o", str(test_object)],
            cwd=root,
            text=True,
            capture_output=True,
        )
        if result.returncode != 0:
            print(f"[BUILD FAIL] {test.parent.name}", file=sys.stderr)
            print(result.stdout, file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1

        result = subprocess.run(
            [compiler, *(str(obj) for obj in objects), str(test_object),
             *(flag for flag in flags if flag.startswith("-fsanitize=")),
             "-o", str(executable)],
            cwd=root,
            text=True,
            capture_output=True,
        )
        if result.returncode != 0:
            print(f"[LINK FAIL] {test.parent.name}", file=sys.stderr)
            print(result.stdout, file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1

        environment = os.environ.copy()
        if args.sanitizers:
            environment["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
            environment["UBSAN_OPTIONS"] = "halt_on_error=1"

        result = subprocess.run(
            [str(executable)],
            cwd=root,
            text=True,
            capture_output=True,
            env=environment,
        )

        print(result.stdout, end="")
        if result.returncode != 0:
            print(f"[TEST FAIL] {test.parent.name}", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1

        print(f"[PASS] {test.parent.name}")
        passed += 1

    shutil.rmtree(build)
    print(f"All {passed} native test suites passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
