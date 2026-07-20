#!/usr/bin/env python3
"""Validate granular library ownership, versions, and declared dependencies."""

from __future__ import annotations

import json
from pathlib import Path
import sys

from library_layout import discover_libraries, framework_includes, header_providers


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    libraries = discover_libraries(root)
    providers = header_providers(libraries)
    root_version = json.loads(
        (root / "library.json").read_text(encoding="utf-8")
    )["version"]

    errors: list[str] = []
    for library in libraries.values():
        if library.version != root_version:
            errors.append(
                f"{library.name}: version {library.version} differs from root {root_version}"
            )
        for dependency in library.dependencies:
            if dependency not in libraries:
                errors.append(f"{library.name}: unknown dependency {dependency}")

        declared = set(library.dependencies)
        for path in (*library.headers(), *library.sources()):
            for include in framework_includes(path):
                provider = providers.get(include)
                if provider is None:
                    errors.append(f"{path}: no provider for <{include}>")
                    continue
                if provider != library.name and provider not in declared:
                    errors.append(
                        f"{library.name}: {path.relative_to(root)} includes <{include}> "
                        f"from {provider} without declaring that dependency"
                    )

    # Every canonical public header must live below libraries/. The obsolete root
    # include tree caused API divergence and must not return.
    if (root / "include").exists():
        errors.append("obsolete root include/ tree exists; public headers must have one owner")

    if errors:
        print("Granular library layout validation failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    source_count = sum(len(library.sources()) for library in libraries.values())
    print(
        f"Validated {len(libraries)} libraries, {len(providers)} public headers, "
        f"and {source_count} implementation files."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
