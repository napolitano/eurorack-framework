#!/usr/bin/env python3
"""Build native tests with GCC coverage and emit JSON/Markdown reports.

The runner supports focused suites, per-suite summaries, line and branch
coverage, and policy thresholds. It intentionally measures only framework
sources below ``libraries/``.
"""
from __future__ import annotations

import argparse
import gzip
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from typing import Any

from library_layout import (
    discover_libraries,
    header_providers,
    include_flags,
    libraries_for_files,
    source_files,
)


def _compile_and_run(root: Path, build: Path, tests: list[Path], compiler: str) -> None:
    libraries = discover_libraries(root)
    providers = header_providers(libraries)
    common = [
        "-std=c++17", "-O0", "-g", "--coverage",
        "-fprofile-abs-path", "-Wall", "-Wextra", "-Wpedantic",
        "-I", str(root / "tools/test_support"),
    ]
    for test in tests:
        suite = test.parent.name
        suite_build = build / suite
        suite_build.mkdir(parents=True)
        inputs = tuple(
            p for p in sorted(test.parent.rglob("*"))
            if p.is_file() and p.suffix in {".h", ".hpp", ".c", ".cc", ".cpp"}
        )
        selected = libraries_for_files(inputs, libraries, providers)
        flags = [*common, *include_flags(libraries, selected), "-I", str(test.parent)]
        sources = (*source_files(libraries, selected), *[p for p in inputs if p.suffix in {".c", ".cc", ".cpp"}])
        objects: list[Path] = []
        for index, source in enumerate(sources):
            obj = suite_build / f"source_{index}.o"
            result = subprocess.run(
                [compiler, *flags, "-c", str(source), "-o", str(obj)],
                cwd=root, text=True, capture_output=True,
            )
            if result.returncode:
                raise RuntimeError(f"coverage build failed for {suite}:\n{result.stdout}{result.stderr}")
            objects.append(obj)
        executable = suite_build / suite
        result = subprocess.run(
            [compiler, *(str(p) for p in objects), "--coverage", "-o", str(executable)],
            cwd=root, text=True, capture_output=True,
        )
        if result.returncode:
            raise RuntimeError(f"coverage link failed for {suite}:\n{result.stdout}{result.stderr}")
        result = subprocess.run([str(executable)], cwd=root, text=True, capture_output=True)
        if result.returncode:
            raise RuntimeError(f"coverage test failed for {suite}:\n{result.stdout}{result.stderr}")
        print(f"[COVERAGE PASS] {suite}: {', '.join(selected)}")


def _gcov_reports(build: Path, gcov: str) -> list[dict[str, Any]]:
    reports: list[dict[str, Any]] = []
    for gcno in sorted(build.rglob("*.gcno")):
        result = subprocess.run(
            [gcov, "--json-format", "--branch-probabilities", "--branch-counts", "-o", str(gcno.parent), str(gcno)],
            cwd=gcno.parent, text=True, capture_output=True,
        )
        if result.returncode not in (0, 1):
            print(result.stderr, file=sys.stderr)
    for report in build.rglob("*.gcov.json.gz"):
        with gzip.open(report, "rt", encoding="utf-8") as handle:
            reports.append(json.load(handle))
    return reports


def _aggregate(root: Path, reports: list[dict[str, Any]]) -> dict[str, Any]:
    files: dict[str, dict[str, int]] = {}
    for report in reports:
        for entry in report.get("files", []):
            raw = Path(entry.get("file", ""))
            try:
                relative = raw.resolve().relative_to(root.resolve())
            except (ValueError, OSError):
                continue
            if not relative.parts or relative.parts[0] != "libraries":
                continue
            key = relative.as_posix()
            item = files.setdefault(key, {"lines": 0, "linesCovered": 0, "branches": 0, "branchesCovered": 0})
            for line in entry.get("lines", []):
                item["lines"] += 1
                if int(line.get("count", 0)) > 0:
                    item["linesCovered"] += 1
                for branch in line.get("branches", []):
                    item["branches"] += 1
                    if int(branch.get("count", 0)) > 0:
                        item["branchesCovered"] += 1
    totals = {"lines": 0, "linesCovered": 0, "branches": 0, "branchesCovered": 0}
    for item in files.values():
        for key in totals:
            totals[key] += item[key]
    def pct(covered: int, total: int) -> float:
        return round(100.0 * covered / total, 2) if total else 0.0
    totals["linePercent"] = pct(totals["linesCovered"], totals["lines"])
    totals["branchPercent"] = pct(totals["branchesCovered"], totals["branches"])
    return {"schemaVersion": 1, "totals": totals, "files": files}


def _write_reports(root: Path, data: dict[str, Any], suites: list[str], policy: dict[str, Any]) -> None:
    out_dir = root / "build/coverage"
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "coverage.json").write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    totals = data["totals"]
    rows = []
    for path, item in sorted(data["files"].items()):
        lp = 100.0 * item["linesCovered"] / item["lines"] if item["lines"] else 100.0
        bp = 100.0 * item["branchesCovered"] / item["branches"] if item["branches"] else 100.0
        rows.append(f"| `{path}` | {item['linesCovered']}/{item['lines']} | {lp:.1f}% | {item['branchesCovered']}/{item['branches']} | {bp:.1f}% |")
    markdown = "\n".join([
        "# Native Coverage", "",
        "Generated by `python tools/run-native-coverage.py`.", "",
        f"- Suites: {', '.join(suites)}",
        f"- Line coverage: {totals['linesCovered']}/{totals['lines']} ({totals['linePercent']:.1f}%)",
        f"- Branch coverage: {totals['branchesCovered']}/{totals['branches']} ({totals['branchPercent']:.1f}%)",
        f"- Policy minimum: {policy['minimum']['line']:.1f}% lines, {policy['minimum']['branch']:.1f}% branches",
        "", "## Files", "",
        "| File | Lines | Line % | Branches | Branch % |",
        "|---|---:|---:|---:|---:|", *rows, "",
        "Coverage complements protocol, sanitizer, compile and hardware tests; it is not a correctness proof.", "",
    ])
    (root / "docs/reference/native-coverage.md").write_text(markdown, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--filter", default="", help="Run suites whose directory name contains this text.")
    parser.add_argument("--driver-suites", action="store_true", help="Run only dedicated test_driver_* suites.")
    parser.add_argument("--no-threshold", action="store_true", help="Generate reports without enforcing policy thresholds.")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    gcov = shutil.which("gcov")
    if compiler is None or gcov is None:
        print("g++ and gcov are required.", file=sys.stderr)
        return 2
    policy = json.loads((root / "tools/coverage-policy.json").read_text(encoding="utf-8"))
    tests = sorted((root / "tests/native").glob("test_*/test_main.cpp"))
    if args.driver_suites:
        tests = [p for p in tests if p.parent.name.startswith("test_driver_")]
    if args.filter:
        tests = [p for p in tests if args.filter in p.parent.name]
    if not tests:
        print("No matching native test suites.", file=sys.stderr)
        return 2
    build = root / ".coverage"
    if build.exists():
        shutil.rmtree(build)
    build.mkdir()
    try:
        _compile_and_run(root, build, tests, compiler)
        data = _aggregate(root, _gcov_reports(build, gcov))
        suites = [p.parent.name for p in tests]
        _write_reports(root, data, suites, policy)
        totals = data["totals"]
        print(f"Native framework line coverage: {totals['linePercent']:.2f}%")
        print(f"Native framework branch coverage: {totals['branchPercent']:.2f}%")
        minimum = policy["driverMinimum"] if args.driver_suites else policy["minimum"]
        if not args.no_threshold and (
            totals["linePercent"] < float(minimum["line"]) or
            totals["branchPercent"] < float(minimum["branch"])
        ):
            print(
                f"Coverage policy failed: expected >= {minimum['line']}% lines and >= {minimum['branch']}% branches.",
                file=sys.stderr,
            )
            return 1
        print("Coverage policy passed.")
        return 0
    except RuntimeError as error:
        print(error, file=sys.stderr)
        return 1
    finally:
        shutil.rmtree(build, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
