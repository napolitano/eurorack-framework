#!/usr/bin/env python3
"""Build Markdown documentation artifacts and an optional Doxygen PDF."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile


DOCUMENTATION_DIRECTORIES = (
    "docs/manual",
    "docs/guides",
    "docs/architecture",
    "docs/release",
)


def run(command: list[str], root: Path) -> None:
    print("+", " ".join(command))
    subprocess.run(command, cwd=root, check=True)


def version(root: Path) -> str:
    metadata = json.loads((root / "library.json").read_text(encoding="utf-8"))
    return str(metadata["version"])


def markdown_files(root: Path) -> list[Path]:
    files: list[Path] = []
    for relative in DOCUMENTATION_DIRECTORIES:
        directory = root / relative
        if directory.exists():
            files.extend(directory.rglob("*.md"))
    files.append(root / "README.md")
    return sorted(set(files), key=lambda item: item.as_posix())


def write_markdown_archive(root: Path, output: Path, release_version: str) -> Path:
    archive_path = output / (
        f"eurorack-framework-{release_version}-markdown-docs.zip"
    )
    with zipfile.ZipFile(
        archive_path,
        "w",
        zipfile.ZIP_DEFLATED,
        compresslevel=9,
    ) as archive:
        for path in markdown_files(root):
            archive.write(path, path.relative_to(root))
    return archive_path


def build_pdf(root: Path, output: Path, release_version: str) -> Path:
    doxygen = shutil.which("doxygen")
    if doxygen is None:
        raise RuntimeError("doxygen was not found on PATH")

    run([doxygen, "Doxyfile"], root)

    latex_directory = root / "docs/generated/latex"
    makefile = latex_directory / "Makefile"
    if not makefile.is_file():
        raise RuntimeError("Doxygen did not create the LaTeX Makefile")

    make = shutil.which("make")
    if make is None:
        raise RuntimeError("make was not found on PATH")

    run([make], latex_directory)

    generated = latex_directory / "refman.pdf"
    if not generated.is_file():
        raise RuntimeError("LaTeX did not create refman.pdf")

    destination = output / (
        f"eurorack-framework-{release_version}-manual.pdf"
    )
    shutil.copy2(generated, destination)
    return destination


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-directory", default="dist")
    parser.add_argument("--markdown-only", action="store_true")
    arguments = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    output = (root / arguments.output_directory).resolve()
    output.mkdir(parents=True, exist_ok=True)
    release_version = version(root)

    artifacts = [
        write_markdown_archive(root, output, release_version)
    ]

    if not arguments.markdown_only:
        artifacts.append(
            build_pdf(root, output, release_version)
        )

    print("Documentation artifacts:")
    for artifact in artifacts:
        print(artifact)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
