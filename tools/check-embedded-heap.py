#!/usr/bin/env python3
"""Reject dynamic allocation in embedded-critical framework libraries."""
from __future__ import annotations

from pathlib import Path
import re
import sys

CRITICAL_LIBRARIES = {
    "eurorack-control-analog-button-ladder",
    "eurorack-control-encoder-acceleration",
    "eurorack-control-event-queue",
    "eurorack-control-press-classifier",
    "eurorack-control-soft-takeover",

    "eurorack-driver-mcp3208",
    "eurorack-driver-mcp4822",
    "eurorack-driver-mcp4922",
    "eurorack-driver-tlc5947",
    "eurorack-platform-avr-adc-scanner",
    "eurorack-platform-avr-external-edge",
    "eurorack-platform-avr-timer2",
    "eurorack-storage-fixed-slot",
}
PATTERNS = {
    "std::vector": re.compile(r"\bstd::vector\b"),
    "new-expression": re.compile(r"\bnew\s+[A-Za-z_][A-Za-z0-9_:<>]*\s*(?:\(|\[)"),
    "malloc": re.compile(r"\bmalloc\s*\("),
    "calloc": re.compile(r"\bcalloc\s*\("),
    "realloc": re.compile(r"\brealloc\s*\("),
    "free": re.compile(r"\bfree\s*\("),
}


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    failures: list[str] = []
    for library in sorted((root / "libraries").iterdir()):
        if not library.is_dir() or library.name not in CRITICAL_LIBRARIES:
            continue
        for source in sorted(library.rglob("*")):
            if source.suffix not in {".h", ".hpp", ".c", ".cc", ".cpp"}:
                continue
            text = source.read_text(encoding="utf-8")
            for label, pattern in PATTERNS.items():
                for match in pattern.finditer(text):
                    line = text.count("\n", 0, match.start()) + 1
                    failures.append(f"{source.relative_to(root)}:{line}: forbidden {label}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("Embedded heap policy passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
