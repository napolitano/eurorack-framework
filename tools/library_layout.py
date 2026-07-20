#!/usr/bin/env python3
"""Discover and resolve the repository's granular PlatformIO libraries."""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
from typing import Iterable

FRAMEWORK_INCLUDE = re.compile(r'^\s*#include\s*[<"](eurorack(?:_config\.hpp|/[^>"]+))[>"]', re.MULTILINE)


@dataclass(frozen=True)
class Library:
    """One library declared below the repository's ``libraries`` directory."""

    name: str
    path: Path
    dependencies: tuple[str, ...]
    include_dir: Path
    source_dir: Path
    version: str

    def headers(self) -> tuple[Path, ...]:
        """Return every public C or C++ header owned by the library."""
        if not self.include_dir.is_dir():
            return ()
        return tuple(
            sorted(
                path
                for path in self.include_dir.rglob("*")
                if path.is_file() and path.suffix in {".h", ".hpp"}
            )
        )

    def sources(self) -> tuple[Path, ...]:
        """Return every non-header implementation owned by the library."""
        if not self.source_dir.is_dir():
            return ()
        return tuple(
            sorted(
                path
                for path in self.source_dir.rglob("*")
                if path.is_file() and path.suffix in {".c", ".cc", ".cpp", ".cxx"}
            )
        )


def discover_libraries(root: Path) -> dict[str, Library]:
    """Read all PlatformIO manifests and return libraries keyed by unique name."""
    result: dict[str, Library] = {}
    for manifest_path in sorted((root / "libraries").glob("*/library.json")):
        metadata = json.loads(manifest_path.read_text(encoding="utf-8"))
        name = metadata.get("name")
        version = metadata.get("version")
        if not isinstance(name, str) or not name:
            raise RuntimeError(f"invalid library name in {manifest_path}")
        if not isinstance(version, str) or not version:
            raise RuntimeError(f"invalid library version in {manifest_path}")
        if name in result:
            raise RuntimeError(f"duplicate library name: {name}")

        build = metadata.get("build", {})
        include_name = build.get("includeDir", "include")
        source_name = build.get("srcDir", "src")
        if not isinstance(include_name, str) or not isinstance(source_name, str):
            raise RuntimeError(f"invalid build directories in {manifest_path}")

        raw_dependencies = metadata.get("dependencies", {})
        if isinstance(raw_dependencies, dict):
            dependencies = tuple(raw_dependencies.keys())
        elif isinstance(raw_dependencies, list):
            dependencies = tuple(
                item["name"] if isinstance(item, dict) else item
                for item in raw_dependencies
            )
        else:
            raise RuntimeError(f"invalid dependencies in {manifest_path}")
        if not all(isinstance(item, str) and item for item in dependencies):
            raise RuntimeError(f"invalid dependency name in {manifest_path}")

        directory = manifest_path.parent
        result[name] = Library(
            name=name,
            path=directory,
            dependencies=dependencies,
            include_dir=directory / include_name,
            source_dir=directory / source_name,
            version=version,
        )
    return result


def header_providers(libraries: dict[str, Library]) -> dict[str, str]:
    """Map each public include spelling to the single library that owns it."""
    providers: dict[str, str] = {}
    for library in libraries.values():
        for header in library.headers():
            relative = header.relative_to(library.include_dir).as_posix()
            previous = providers.get(relative)
            if previous is not None:
                raise RuntimeError(
                    f"public header {relative} is provided by both {previous} and {library.name}"
                )
            providers[relative] = library.name
    return providers


def framework_includes(path: Path) -> tuple[str, ...]:
    """Return public framework include spellings found in one source file."""
    text = path.read_text(encoding="utf-8", errors="replace")
    return tuple(match.group(1) for match in FRAMEWORK_INCLUDE.finditer(text))


def dependency_closure(
    initial: Iterable[str], libraries: dict[str, Library]
) -> tuple[str, ...]:
    """Resolve a deterministic transitive dependency closure."""
    pending = list(initial)
    resolved: set[str] = set()
    while pending:
        name = pending.pop()
        if name in resolved:
            continue
        library = libraries.get(name)
        if library is None:
            raise RuntimeError(f"unknown library dependency: {name}")
        resolved.add(name)
        pending.extend(library.dependencies)
    return tuple(sorted(resolved))


def libraries_for_files(
    paths: Iterable[Path], libraries: dict[str, Library], providers: dict[str, str]
) -> tuple[str, ...]:
    """Resolve the libraries required by framework includes in source files."""
    direct: set[str] = set()
    for path in paths:
        for include in framework_includes(path):
            provider = providers.get(include)
            if provider is None:
                raise RuntimeError(f"no library provides <{include}> included by {path}")
            direct.add(provider)
    return dependency_closure(direct, libraries)


def include_flags(libraries: dict[str, Library], names: Iterable[str]) -> list[str]:
    """Create compiler include flags for a resolved library set."""
    flags: list[str] = []
    for name in sorted(set(names)):
        include_dir = libraries[name].include_dir
        if include_dir.is_dir():
            flags.extend(["-I", str(include_dir)])
    return flags


def source_files(libraries: dict[str, Library], names: Iterable[str]) -> tuple[Path, ...]:
    """Return unique implementation files owned by a resolved library set."""
    return tuple(
        sorted(
            {
                source
                for name in names
                for source in libraries[name].sources()
            }
        )
    )
