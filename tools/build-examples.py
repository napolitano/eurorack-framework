#!/usr/bin/env python3
"""Compile and execute native component examples with granular dependencies."""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import tempfile

from library_layout import (
    discover_libraries,
    header_providers,
    include_flags,
    libraries_for_files,
    source_files,
)


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
    libraries = discover_libraries(root)
    providers = header_providers(libraries)

    directories = sorted(
        path
        for path in examples_root.iterdir()
        if path.is_dir() and (path / "main.cpp").is_file()
    )
    if args.example:
        directories = [path for path in directories if path.name == args.example]
        if not directories:
            raise RuntimeError(f"unknown example: {args.example}")

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
        "-Werror",
    ]

    with tempfile.TemporaryDirectory(prefix="eurorack-framework-examples-") as temporary:
        output = Path(temporary)

        for directory in directories:
            local_sources = tuple(sorted(directory.rglob("*.cpp")))
            local_inputs = tuple(sorted((*directory.rglob("*.cpp"), *directory.rglob("*.hpp"), examples_root / "driver_example_support.hpp")))
            selected = libraries_for_files(local_inputs, libraries, providers)
            flags = [*common_flags, *include_flags(libraries, selected), "-I", str(examples_root), "-I", str(directory)]
            objects: list[str] = []

            example_objects = output / f"{directory.name}-objects"
            example_objects.mkdir(parents=True)
            all_sources = (*source_files(libraries, selected), *local_sources)
            for index, source in enumerate(all_sources):
                object_file = example_objects / f"{index}.o"
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
                objects.append(str(object_file))

            executable = output / directory.name
            run([args.compiler, *objects, "-o", str(executable)], root)
            run([str(executable)], root)
            print(f"[PASS] {directory.name}: {', '.join(selected)}")

    print(f"All {len(directories)} component examples passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
