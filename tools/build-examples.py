#!/usr/bin/env python3
"""Compile and execute all native component examples."""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import tempfile


def run(command: list[str], root: Path) -> None:
    """Run one compiler or example command and fail immediately on errors."""
    print("+", " ".join(command))
    subprocess.run(command, cwd=root, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--example",
        help="Build only the named examples/components subdirectory.",
    )
    parser.add_argument(
        "--compiler",
        default="g++",
        help="C++17 compiler executable. Default: g++.",
    )
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    examples_root = root / "examples/components"

    directories = sorted(
        path
        for path in examples_root.iterdir()
        if path.is_dir() and (path / "main.cpp").is_file()
    )
    if args.example:
        directories = [
            path for path in directories if path.name == args.example
        ]
        if not directories:
            raise RuntimeError(f"unknown example: {args.example}")

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
        "-Werror",
        "-I",
        str(root / "include"),
    ]

    # A temporary build directory avoids stale binaries, repository pollution,
    # and permission problems caused by artifacts created by another user or CI.
    with tempfile.TemporaryDirectory(
        prefix="eurorack-framework-examples-"
    ) as temporary:
        output = Path(temporary)
        objects = output / "objects"
        objects.mkdir(parents=True)

        object_files: list[str] = []
        for index, source in enumerate(
            sorted((root / "src").rglob("*.cpp"))
        ):
            object_file = objects / f"{index}.o"
            run(
                [
                    args.compiler,
                    *flags,
                    "-c",
                    str(source),
                    "-o",
                    str(object_file),
                ],
                root,
            )
            object_files.append(str(object_file))

        for directory in directories:
            executable = output / directory.name
            run(
                [
                    args.compiler,
                    *flags,
                    str(directory / "main.cpp"),
                    *object_files,
                    "-o",
                    str(executable),
                ],
                root,
            )
            run([str(executable)], root)
            print(f"[PASS] {directory.name}")

    print(f"All {len(directories)} component examples passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
