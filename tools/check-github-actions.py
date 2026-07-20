#!/usr/bin/env python3
"""Perform dependency-free structural checks on GitHub Actions workflows."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
WORKFLOWS = ROOT / ".github" / "workflows"
REQUIRED_KEYS = ("name:", "on:", "jobs:")
PINNED_ACTIONS = {
    "actions/checkout": "v6",
    "actions/setup-python": "v6",
    "actions/upload-artifact": "v7",
}

def main() -> int:
    errors: list[str] = []
    files = sorted(WORKFLOWS.glob("*.yml"))
    if not files:
        errors.append("No workflow files found.")
    for path in files:
        text = path.read_text(encoding="utf-8")
        for key in REQUIRED_KEYS:
            if not re.search(rf"(?m)^{re.escape(key)}", text):
                errors.append(f"{path.relative_to(ROOT)}: missing top-level {key}")
        if "ubuntu-latest" in text:
            errors.append(f"{path.relative_to(ROOT)}: use explicit ubuntu-24.04 runner")
        for action, version in PINNED_ACTIONS.items():
            for match in re.finditer(rf"{re.escape(action)}@(v\d+)", text):
                if match.group(1) != version:
                    errors.append(
                        f"{path.relative_to(ROOT)}: {action} must use {version}, found {match.group(1)}"
                    )
        if "pull_request_target:" in text:
            errors.append(f"{path.relative_to(ROOT)}: pull_request_target is forbidden")
        if re.search(r"permissions:\s*\n\s+contents:\s+write", text):
            errors.append(f"{path.relative_to(ROOT)}: write permission requires explicit review")
    if errors:
        print("GitHub Actions validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"GitHub Actions validation passed for {len(files)} workflows.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
