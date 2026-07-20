#!/usr/bin/env python3
"""Run deterministic framework release checks."""
from __future__ import annotations
import argparse
from pathlib import Path
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--coverage",
        action="store_true",
        help="Also run the slower dedicated-driver coverage gate.",
    )
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    commands = [
        ["python", "tools/generate-framework-map.py"],
        ["python", "tools/generate-driver-test-matrix.py"],
        ["python", "tools/check-library-layout.py"],
        ["python", "tools/check-documentation-coverage.py"],
        ["python", "tools/check-hardware-docs.py"],
        ["python", "tools/check-driver-library-docs.py"],
        ["python", "tools/check-embedded-heap.py"],
        ["python", "tools/check-public-docs.py"],
        ["python", "tools/check-maintainability-docs.py"],
        ["python", "tools/check-doxygen-contracts.py"],
        ["python", "tools/check-licensing.py"],
        ["python", "tools/check-resource-conflicts.py", "examples/profiles/avr-shared-spi.json"],
        ["python", "tools/build-examples.py"],
        ["python", "tools/run-native-tests.py"],
    ]
    if args.coverage:
        commands.append(["python", "tools/run-native-coverage.py", "--driver-suites"])
    for command in commands:
        print("> " + " ".join(command), flush=True)
        result = subprocess.run(command, cwd=root)
        if result.returncode:
            return result.returncode
    print("Release preflight passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
