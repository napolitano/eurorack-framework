#!/usr/bin/env python3
"""Validate the framework and create reproducible release artifacts."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile


EXCLUDED_PARTS = {
    ".git",
    ".cache",
    ".idea",
    ".pio",
    ".vscode",
    ".venv",
    ".native-tests",
    ".validation",
    ".validation",
    "__pycache__",
    "dist",
    "generated",
    "build",
}


def run(command: list[str], root: Path) -> None:
    """Run one required command and stop on failure."""
    print("+", " ".join(command))
    subprocess.run(command, cwd=root, check=True)


def version(root: Path) -> str:
    """Read the package version from library.json."""
    metadata = json.loads((root / "library.json").read_text(encoding="utf-8"))
    value = metadata.get("version")
    if not isinstance(value, str) or not value:
        raise RuntimeError("library.json does not contain a valid version")
    return value


def source_files(root: Path) -> list[Path]:
    """Return sorted release source files."""
    result: list[Path] = []
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if any(part in EXCLUDED_PARTS for part in relative.parts):
            continue
        if path.suffix in {".pyc", ".tmp"}:
            continue
        if relative.name in {"compile_commands.json", ".DS_Store"}:
            continue
        result.append(path)
    return sorted(result, key=lambda item: item.as_posix())


def write_zip(archive_path: Path, root: Path, files: list[Path], prefix: str) -> None:
    """Write a deterministic ZIP archive."""
    temporary = archive_path.with_suffix(archive_path.suffix + ".tmp")
    if temporary.exists():
        temporary.unlink()

    with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in files:
            relative = path.relative_to(root)
            information = zipfile.ZipInfo(f"{prefix}/{relative.as_posix()}")
            information.date_time = (2026, 1, 1, 0, 0, 0)
            information.compress_type = zipfile.ZIP_DEFLATED
            information.external_attr = 0o644 << 16
            archive.writestr(information, path.read_bytes())

    temporary.replace(archive_path)


def sha256(path: Path) -> str:
    """Return the SHA-256 digest of one file."""
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--output-directory", default="dist")
    parser.add_argument("--skip-tests", action="store_true")
    parser.add_argument("--skip-documentation", action="store_true")
    parser.add_argument("--markdown-only", action="store_true")
    arguments = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    output = (root / arguments.output_directory).resolve()
    output.mkdir(parents=True, exist_ok=True)
    release_version = version(root)
    base_name = f"eurorack-framework-{release_version}"

    run([sys.executable, "tools/check-library-layout.py"], root)

    if not arguments.skip_tests:
        run(
            [
                sys.executable,
                "tools/run-native-tests.py",
                "--compiler",
                arguments.compiler,
            ],
            root,
        )

    run([sys.executable, "tools/check-public-docs.py"], root)

    documentation_artifacts: list[Path] = []
    if not arguments.skip_documentation:
        documentation_command = [
            sys.executable,
            "tools/build-documentation.py",
            "--output-directory",
            str(output),
        ]
        if arguments.markdown_only:
            documentation_command.append("--markdown-only")
        run(documentation_command, root)

        markdown_archive = output / (
            f"{base_name}-markdown-docs.zip"
        )
        if not markdown_archive.is_file():
            raise RuntimeError(
                "Markdown documentation archive was not created"
            )
        documentation_artifacts.append(markdown_archive)

        pdf_manual = output / f"{base_name}-manual.pdf"
        if not arguments.markdown_only:
            if not pdf_manual.is_file():
                raise RuntimeError("PDF manual was not created")
            documentation_artifacts.append(pdf_manual)

    source_archive = output / f"{base_name}-source.zip"
    write_zip(source_archive, root, source_files(root), base_name)
    run([sys.executable, "tools/validate-release-archive.py", str(source_archive)], root)

    artifacts = [source_archive, *documentation_artifacts]

    checksum_path = output / f"{base_name}-checksums.sha256"
    checksum_path.write_text(
        "".join(f"{sha256(path)}  {path.name}\n" for path in artifacts),
        encoding="utf-8",
    )

    print("Artifacts:")
    for path in [*artifacts, checksum_path]:
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
