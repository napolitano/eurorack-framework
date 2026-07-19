#!/usr/bin/env python3
"""Reject Doxygen contracts that can merge into duplicate API documentation."""

from __future__ import annotations

from pathlib import Path
import re
import sys


DOCUMENTING_MARKERS = ("/**", "/*!", "///", "//!")
API_COMMAND_PATTERN = re.compile(
    r"@(brief|param|return|retval|throws?|exception|p)\b"
)
PARAM_PATTERN = re.compile(
    r"@param(?:\[[^\]]+\])?\s+([A-Za-z_][A-Za-z0-9_]*)"
)
RETURN_PATTERN = re.compile(r"@return\b")


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def audit_implementation_comments(root: Path) -> list[str]:
    failures: list[str] = []

    for path in sorted((root / "libraries").rglob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        header_end = text.find("*/")

        if header_end < 0:
            failures.append(
                f"{path.relative_to(root)}: missing file-level Doxygen header"
            )
            continue

        body_offset = header_end + 2
        body = text[body_offset:]

        for marker in DOCUMENTING_MARKERS:
            marker_offset = body.find(marker)
            if marker_offset >= 0:
                failures.append(
                    f"{path.relative_to(root)}:"
                    f"{line_number(text, body_offset + marker_offset)}: "
                    "implementation-level Doxygen comment found; public "
                    "contracts belong in declarations under include/"
                )

        for match in API_COMMAND_PATTERN.finditer(body):
            failures.append(
                f"{path.relative_to(root)}:"
                f"{line_number(text, body_offset + match.start())}: "
                f"implementation comment still contains @{match.group(1)}"
            )

    return failures


def audit_public_blocks(root: Path) -> list[str]:
    failures: list[str] = []

    for path in sorted((root / "libraries").rglob("*")):
        if not path.is_file() or path.suffix not in {".h", ".hpp"}:
            continue

        text = path.read_text(encoding="utf-8")
        for block_match in re.finditer(r"/\*\*(.*?)\*/", text, re.S):
            block = block_match.group(1)
            block_line = line_number(text, block_match.start())

            parameters = PARAM_PATTERN.findall(block)
            repeated = sorted(
                name
                for name in set(parameters)
                if parameters.count(name) > 1
            )
            for name in repeated:
                failures.append(
                    f"{path.relative_to(root)}:{block_line}: "
                    f"parameter {name!r} has multiple @param sections"
                )

            return_count = len(RETURN_PATTERN.findall(block))
            if return_count > 1:
                failures.append(
                    f"{path.relative_to(root)}:{block_line}: "
                    f"Doxygen block has {return_count} @return sections"
                )

    return failures


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    failures = [
        *audit_implementation_comments(root),
        *audit_public_blocks(root),
    ]

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1

    print("Doxygen contract audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
