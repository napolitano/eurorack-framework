#!/usr/bin/env python3
"""Build Markdown documentation and the optional combined Doxygen PDF manual."""

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


class DocumentationBuildError(RuntimeError):
    """Raised when one documentation build stage fails."""


def run_logged(
    command: list[str],
    cwd: Path,
    log_path: Path,
) -> None:
    """Run one command, mirror output, and retain a diagnostic log."""
    print("+", " ".join(command))
    result = subprocess.run(
        command,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )

    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text(result.stdout, encoding="utf-8")
    print(result.stdout, end="")

    if result.returncode != 0:
        raise DocumentationBuildError(
            f"{' '.join(command)} failed with exit code "
            f"{result.returncode}. See {log_path}."
        )


def package_version(root: Path) -> str:
    """Read the development artifact identifier from library.json."""
    metadata = json.loads(
        (root / "library.json").read_text(encoding="utf-8")
    )
    return str(metadata["version"])


def markdown_files(root: Path) -> list[Path]:
    """Return every maintained Markdown documentation source."""
    files: list[Path] = []

    for relative in DOCUMENTATION_DIRECTORIES:
        directory = root / relative
        if directory.exists():
            files.extend(directory.rglob("*.md"))

    files.extend(
        path
        for path in (
            root / "README.md",
            root / "LICENSING.md",
            root / "CONTRIBUTING.md",
        )
        if path.is_file()
    )

    return sorted(set(files), key=lambda item: item.as_posix())


def write_markdown_archive(
    root: Path,
    output: Path,
    version: str,
) -> Path:
    """Write the maintained Markdown documentation as a ZIP archive."""
    archive_path = output / (
        f"eurorack-framework-{version}-markdown-docs.zip"
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


def require_executable(name: str) -> str:
    """Return an executable path or raise an actionable error."""
    executable = shutil.which(name)
    if executable is None:
        raise DocumentationBuildError(
            f"{name} was not found on PATH"
        )
    return executable


def build_pdf(
    root: Path,
    output: Path,
    version: str,
) -> Path:
    """Generate Doxygen LaTeX and compile it with latexmk."""
    doxygen = require_executable("doxygen")
    latexmk = require_executable("latexmk")
    require_executable("pdflatex")
    require_executable("makeindex")
    require_executable("dot")

    generated_root = root / "docs/generated"
    latex_directory = generated_root / "latex"
    logs = generated_root / "logs"

    if generated_root.exists():
        shutil.rmtree(generated_root)
    logs.mkdir(parents=True)

    run_logged(
        [doxygen, "Doxyfile"],
        root,
        logs / "doxygen.log",
    )

    refman = latex_directory / "refman.tex"
    if not refman.is_file():
        raise DocumentationBuildError(
            "Doxygen did not create docs/generated/latex/refman.tex"
        )

    # latexmk handles the repeated PDFLaTeX and makeindex passes that the
    # generated document requires. -halt-on-error prevents a superficially
    # successful PDF from masking a fatal LaTeX problem.
    run_logged(
        [
            latexmk,
            "-pdf",
            "-interaction=nonstopmode",
            "-halt-on-error",
            "-file-line-error",
            "refman.tex",
        ],
        latex_directory,
        logs / "latexmk.log",
    )

    generated_pdf = latex_directory / "refman.pdf"
    if not generated_pdf.is_file():
        raise DocumentationBuildError(
            "LaTeX completed without creating refman.pdf"
        )

    destination = output / (
        f"eurorack-framework-{version}-manual.pdf"
    )
    shutil.copy2(generated_pdf, destination)
    return destination


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-directory", default="dist")
    parser.add_argument("--markdown-only", action="store_true")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    output = (root / args.output_directory).resolve()
    output.mkdir(parents=True, exist_ok=True)
    version = package_version(root)

    artifacts = [
        write_markdown_archive(root, output, version)
    ]

    try:
        if not args.markdown_only:
            artifacts.append(build_pdf(root, output, version))
    except DocumentationBuildError as error:
        print(f"Documentation build failed: {error}", file=sys.stderr)
        return 1

    print("Documentation artifacts:")
    for artifact in artifacts:
        print(artifact)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
