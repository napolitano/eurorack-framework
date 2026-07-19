#!/usr/bin/env python3
"""Reject obvious placeholder documentation and undocumented implementation files."""

from __future__ import annotations

from pathlib import Path
import re
import sys


PLACEHOLDERS = (
    "Public class",
    "Public data structure",
    "Public enumeration",
    "Public interface",
    "Returns current state.",
    "Operation result.",
)


def doc_word_count(text: str) -> int:
    blocks = re.findall(r"/\*\*(.*?)\*/", text, flags=re.S)
    return sum(
        len(re.findall(r"\b[A-Za-z][A-Za-z0-9_-]*\b", block))
        for block in blocks
    )


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    failures: list[str] = []

    for path in sorted((root / "include").rglob("*.hpp")):
        text = path.read_text(encoding="utf-8")
        for placeholder in PLACEHOLDERS:
            if placeholder in text:
                failures.append(
                    f"{path}: placeholder documentation contains {placeholder!r}"
                )

    for path in sorted((root / "src").rglob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        lines = max(1, len(text.splitlines()))
        words = doc_word_count(text)

        if "@details" not in text.split("*/", 1)[0]:
            failures.append(f"{path}: file header has no @details section")

        if lines >= 80 and words < 45:
            failures.append(
                f"{path}: only {words} documentation words for {lines} lines"
            )

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1

    print("Maintainability documentation audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
