#!/usr/bin/env python3
"""Audit repository licensing metadata and required notices."""

from __future__ import annotations

import json
from pathlib import Path
import sys


SPDX_ID = "PolyForm-Noncommercial-1.0.0"
REQUIRED_FILES = (
    "LICENSE",
    "LICENSES/PolyForm-Noncommercial-1.0.0.txt",
    "LICENSING.md",
    "ADDITIONAL_PERMISSION.md",
    "COMMERCIAL_LICENSE.md",
    "NOTICE",
    "CONTRIBUTING.md",
)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    failures: list[str] = []

    for relative in REQUIRED_FILES:
        path = root / relative
        if not path.is_file():
            failures.append(f"missing required licensing file: {relative}")

    root_license = root / "LICENSE"
    reuse_license = root / "LICENSES/PolyForm-Noncommercial-1.0.0.txt"
    if root_license.is_file() and reuse_license.is_file():
        if root_license.read_bytes() != reuse_license.read_bytes():
            failures.append(
                "LICENSE and LICENSES/PolyForm-Noncommercial-1.0.0.txt differ"
            )

    metadata_path = root / "library.json"
    if not metadata_path.is_file():
        failures.append("missing library.json")
    else:
        metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        if metadata.get("license") != SPDX_ID:
            failures.append(
                f"library.json license must be {SPDX_ID!r}"
            )

    for directory in (root / "include", root / "src"):
        for path in sorted(directory.rglob("*")):
            if not path.is_file() or path.suffix not in {".h", ".hpp", ".c", ".cpp"}:
                continue
            text = path.read_text(encoding="utf-8")
            expected = f"SPDX-License-Identifier: {SPDX_ID}"
            if expected not in text:
                failures.append(
                    f"{path.relative_to(root)}: missing {expected}"
                )

    notice = root / "NOTICE"
    if notice.is_file():
        notice_text = notice.read_text(encoding="utf-8")
        required_notice_lines = (
            "Required Notice: Copyright (c) 2026 Axel Napolitano",
            "Required Notice: Original project: "
            "https://github.com/napolitano/eurorack-framework",
            "Required Notice: Licensed under the PolyForm Noncommercial "
            "License 1.0.0 with the Five-Unit Cost-Recovery Permission.",
            "Required Notice: Commercial use requires prior written "
            "permission from Axel Napolitano.",
        )
        for line in required_notice_lines:
            if line not in notice_text:
                failures.append(f"NOTICE missing line: {line}")

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1

    print("Licensing audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
