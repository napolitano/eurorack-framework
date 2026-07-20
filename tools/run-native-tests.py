#!/usr/bin/env python3
"""Build and run every native test against its granular dependency closure."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

from library_layout import (
    discover_libraries,
    header_providers,
    include_flags,
    libraries_for_files,
    source_files,
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--sanitizers", action="store_true")
    parser.add_argument("--filter", default="", help="Run suites whose directory name contains this text.")
    parser.add_argument("--driver-suites", action="store_true", help="Run only dedicated test_driver_* suites.")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    compiler = shutil.which(args.compiler)
    if compiler is None:
        print(f"Compiler not found: {args.compiler}", file=sys.stderr)
        return 2

    libraries = discover_libraries(root)
    providers = header_providers(libraries)
    build = root / ".native-tests"
    if build.exists():
        shutil.rmtree(build)
    build.mkdir()

    common_flags = [
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
        str(root / "tools/test_support"),
    ]

    if args.sanitizers:
        common_flags += [
            "-O1",
            "-g",
            "-fno-omit-frame-pointer",
            "-fsanitize=address,undefined",
        ]
    else:
        common_flags += ["-O2"]

    tests = sorted((root / "tests/native").glob("test_*/test_main.cpp"))
    if args.driver_suites:
        tests = [test for test in tests if test.parent.name.startswith("test_driver_")]
    if args.filter:
        tests = [test for test in tests if args.filter in test.parent.name]
    passed = 0
    for test in tests:
        test_directory = test.parent
        inputs = tuple(
            sorted(
                path
                for path in test_directory.rglob("*")
                if path.is_file() and path.suffix in {".h", ".hpp", ".c", ".cc", ".cpp"}
            )
        )
        selected = libraries_for_files(inputs, libraries, providers)
        flags = [*common_flags, *include_flags(libraries, selected), "-I", str(test_directory)]
        test_build = build / test_directory.name
        test_build.mkdir()
        objects: list[Path] = []

        sources = (*source_files(libraries, selected), *[p for p in inputs if p.suffix in {".c", ".cc", ".cpp"}])
        for index, source in enumerate(sources):
            object_path = test_build / f"source_{index}.o"
            result = subprocess.run(
                [compiler, *flags, "-c", str(source), "-o", str(object_path)],
                cwd=root,
                text=True,
                capture_output=True,
            )
            if result.returncode != 0:
                print(f"[BUILD FAIL] {test_directory.name}: {source}", file=sys.stderr)
                print(result.stdout, file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return 1
            objects.append(object_path)

        executable = test_build / test_directory.name
        linker_flags = [flag for flag in common_flags if flag.startswith("-fsanitize=")]
        result = subprocess.run(
            [compiler, *(str(obj) for obj in objects), *linker_flags, "-o", str(executable)],
            cwd=root,
            text=True,
            capture_output=True,
        )
        if result.returncode != 0:
            print(f"[LINK FAIL] {test_directory.name}", file=sys.stderr)
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
            print(f"[TEST FAIL] {test_directory.name}", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1

        print(f"[PASS] {test_directory.name}: {', '.join(selected)}")
        passed += 1

    shutil.rmtree(build)
    print(f"All {passed} native test suites passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
