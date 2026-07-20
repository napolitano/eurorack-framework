#!/usr/bin/env python3
"""Validate the contents and reproducibility properties of a source ZIP."""
from __future__ import annotations
import argparse
from pathlib import PurePosixPath, Path
import zipfile
import sys

FORBIDDEN_PARTS = {".git", ".pio", "dist", "build", "generated", "__pycache__", ".native-tests"}
FORBIDDEN_SUFFIXES = {".tmp", ".bak", ".orig", ".rej", ".pyc", ".pyo", ".o", ".a", ".elf", ".hex", ".bin"}
REQUIRED = {"README.md", "CHANGELOG.md", "LICENSE", "library.json", "platformio.ini", "tools/release-check.py"}
FIXED_TIME = (2026, 1, 1, 0, 0, 0)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive")
    args = parser.parse_args()
    archive_path = Path(args.archive)
    errors: list[str] = []
    with zipfile.ZipFile(archive_path) as archive:
        infos = archive.infolist()
        names = [info.filename for info in infos if not info.is_dir()]
        if names != sorted(names):
            errors.append("entries are not lexicographically ordered")
        roots = {PurePosixPath(name).parts[0] for name in names}
        if len(roots) != 1:
            errors.append("archive must have exactly one root directory")
            root = ""
        else:
            root = next(iter(roots))
        relative_names = {str(PurePosixPath(name).relative_to(root)) for name in names} if root else set()
        for required in REQUIRED:
            if required not in relative_names:
                errors.append(f"missing required file: {required}")
        for info in infos:
            path = PurePosixPath(info.filename)
            if any(part in FORBIDDEN_PARTS for part in path.parts):
                errors.append(f"forbidden path: {info.filename}")
            if path.suffix in FORBIDDEN_SUFFIXES or path.name.endswith("~"):
                errors.append(f"forbidden file: {info.filename}")
            if not info.is_dir() and info.date_time != FIXED_TIME:
                errors.append(f"non-deterministic timestamp: {info.filename}")
            if not info.is_dir():
                try:
                    archive.read(info).decode("utf-8") if path.suffix in {".md", ".py", ".json", ".hpp", ".cpp", ".h", ".c", ".ini", ".yml", ".yaml"} else None
                except UnicodeDecodeError:
                    errors.append(f"non-UTF-8 text file: {info.filename}")
    if errors:
        print("Release archive validation failed:", file=sys.stderr)
        for error in sorted(set(errors)):
            print(f"- {error}", file=sys.stderr)
        return 1
    print(f"Release archive validation passed: {archive_path}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
