#!/usr/bin/env python3
"""Run available static analyzers and a strict compiler analysis fallback."""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
from library_layout import discover_libraries, include_flags, source_files


def run(command: list[str], root: Path) -> int:
    print("> " + " ".join(command), flush=True)
    return subprocess.run(command, cwd=root).returncode


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--require-external", action="store_true", help="Fail when neither clang-tidy nor cppcheck is installed.")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    libraries = discover_libraries(root)
    target_specific_prefixes = ("eurorack-platform-avr-", "eurorack-platform-arduino")
    selected = {name for name in libraries if not name.startswith(target_specific_prefixes)}
    analyzed = {name for name in selected if name != "eurorack-compat"}
    sources = list(source_files(libraries, analyzed))
    headers = sorted(path for name, library in libraries.items() if name in analyzed for path in library.include_dir.rglob("*.hpp"))
    gxx = shutil.which("g++")
    if not gxx:
        print("g++ not installed.", file=sys.stderr)
        return 2
    flags = ["-std=c++17", "-fsyntax-only", "-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Wsign-conversion", "-Wshadow", "-Wundef", "-Wformat=2", "-Wnull-dereference", "-Wdouble-promotion", "-Werror", *include_flags(libraries, selected)]
    for path in sources:
        if run([gxx, *flags, str(path)], root):
            return 1
    # Header self-containment: compile each public header as a translation unit.
    for path in headers:
        wrapper = f'#include "{path.relative_to(root).as_posix()}"\nint main() {{ return 0; }}\n'
        result = subprocess.run([gxx, *flags, "-I", str(root), "-x", "c++", "-"], cwd=root, input=wrapper, text=True, capture_output=True)
        if result.returncode:
            print(f"Header analysis failed: {path.relative_to(root)}", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1

    external = 0
    cppcheck = shutil.which("cppcheck")
    if cppcheck:
        external += 1
        command = [cppcheck, "--enable=warning,style,performance,portability", "--error-exitcode=1", "--std=c++17", "--inline-suppr", "--quiet", *[f"-I{p}" for p in sorted({str(lib.include_dir) for lib in libraries.values()})], *[str(p) for p in sources]]
        if run(command, root):
            return 1
    clang_tidy = shutil.which("clang-tidy")
    if clang_tidy:
        external += 1
        # Use a conservative analyzer subset without requiring a compilation database.
        checks = "clang-analyzer-*,bugprone-*,performance-*,-bugprone-easily-swappable-parameters"
        for path in sources:
            if run([clang_tidy, str(path), f"-checks={checks}", "--", "-std=c++17", *include_flags(libraries, selected)], root):
                return 1
    if external == 0:
        message = "clang-tidy/cppcheck not installed; strict compiler and header self-containment analysis passed."
        if args.require_external:
            print(message, file=sys.stderr)
            return 2
        print(message)
    skipped = len(libraries) - len(analyzed)
    print(f"Static analysis passed for {len(sources)} sources and {len(headers)} public headers; {skipped} target-specific libraries deferred to target analysis.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
