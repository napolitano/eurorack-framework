#!/usr/bin/env python3
"""Portable formatting gate with optional clang-format verification."""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

TEXT_SUFFIXES = {".h", ".hpp", ".c", ".cc", ".cpp", ".py", ".md", ".json", ".yml", ".yaml", ".ini"}
CPP_SUFFIXES = {".h", ".hpp", ".c", ".cc", ".cpp"}
SKIP_PARTS = {".git", ".pio", "dist", "build", "generated", ".native-tests", "__pycache__"}


def files(root: Path) -> list[Path]:
    return sorted(p for p in root.rglob("*") if p.is_file() and p.suffix in TEXT_SUFFIXES and not any(part in SKIP_PARTS for part in p.relative_to(root).parts))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--require-clang-format", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    violations: list[str] = []
    candidates = files(root)
    for path in candidates:
        raw = path.read_bytes()
        if b"\r\n" in raw:
            violations.append(f"CRLF line endings: {path.relative_to(root)}")
        if raw and not raw.endswith(b"\n"):
            violations.append(f"missing final newline: {path.relative_to(root)}")
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError:
            violations.append(f"not UTF-8: {path.relative_to(root)}")
            continue
        for number, line in enumerate(text.splitlines(), 1):
            if line.rstrip(" \t") != line:
                violations.append(f"trailing whitespace: {path.relative_to(root)}:{number}")

    clang_format = shutil.which("clang-format")
    if clang_format:
        cpp_files = [str(p) for p in candidates if p.suffix in CPP_SUFFIXES]
        for start in range(0, len(cpp_files), 100):
            result = subprocess.run([clang_format, "--dry-run", "--Werror", *cpp_files[start:start+100]], cwd=root, text=True, capture_output=True)
            if result.returncode:
                violations.append("clang-format differences:\n" + result.stderr.strip())
    elif args.require_clang_format:
        print("clang-format is required but not installed.", file=sys.stderr)
        return 2
    else:
        print("clang-format not installed; portable whitespace/encoding checks only.")

    if violations:
        print("Format check failed:", file=sys.stderr)
        for item in violations[:100]:
            print(f"- {item}", file=sys.stderr)
        return 1
    print(f"Format check passed for {len(candidates)} text files.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
