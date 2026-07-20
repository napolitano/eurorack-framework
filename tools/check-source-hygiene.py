#!/usr/bin/env python3
"""Reject generated, temporary, editor, and build debris in the source tree."""
from __future__ import annotations
from pathlib import Path
import sys

FORBIDDEN_DIRS = {"__pycache__", ".native-tests", ".coverage", ".pytest_cache", ".mypy_cache", ".cache"}
FORBIDDEN_SUFFIXES = {".tmp", ".bak", ".orig", ".rej", ".pyc", ".pyo", ".swp", ".swo", ".o", ".a", ".elf", ".hex", ".bin"}
FORBIDDEN_NAMES = {".DS_Store", "Thumbs.db", "compile_commands.json"}


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    violations: list[str] = []
    for path in root.rglob("*"):
        relative = path.relative_to(root)
        if any(part in {".git", ".pio", "dist", "build", "generated"} for part in relative.parts):
            continue
        if path.is_dir() and path.name in FORBIDDEN_DIRS:
            violations.append(f"forbidden directory: {relative}")
        elif path.is_file() and (path.name in FORBIDDEN_NAMES or path.suffix in FORBIDDEN_SUFFIXES or path.name.endswith("~")):
            violations.append(f"forbidden file: {relative}")
    if violations:
        print("Source hygiene check failed:", file=sys.stderr)
        for item in sorted(set(violations)):
            print(f"- {item}", file=sys.stderr)
        return 1
    print("Source hygiene check passed.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
