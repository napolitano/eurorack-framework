#!/usr/bin/env python3
"""Audit public C++ headers for complete Doxygen documentation."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import sys


@dataclass
class Declaration:
    path: Path
    line: int
    text: str
    documentation: str


def split_parameters(text: str) -> list[str]:
    """Split a C++ parameter list without splitting nested templates."""
    result: list[str] = []
    current: list[str] = []
    depth = 0

    for character in text:
        if character in "(<[{":
            depth += 1
        elif character in ")>]}":
            depth = max(0, depth - 1)

        if character == "," and depth == 0:
            result.append("".join(current).strip())
            current = []
        else:
            current.append(character)

    if current:
        result.append("".join(current).strip())

    return [item for item in result if item and item != "void"]


def parameter_name(parameter: str) -> str | None:
    """Extract a named parameter from a normal framework declaration."""
    declaration = parameter.split("=", 1)[0].strip()
    identifiers = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", declaration)
    if not identifiers:
        return None

    candidate = identifiers[-1]
    type_only = {
        "bool",
        "char",
        "const",
        "double",
        "float",
        "int",
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
        "long",
        "noexcept",
        "override",
        "short",
        "size_t",
        "std",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "unsigned",
        "void",
    }

    return None if candidate in type_only else candidate


def preceding_doc(lines: list[str], declaration_line: int) -> str:
    """Return the Doxygen block immediately preceding a declaration."""
    index = declaration_line - 1
    inspected = 0
    while index >= 0 and inspected < 6:
        stripped = lines[index].strip()
        if stripped.endswith("*/"):
            break
        if (
            not stripped
            or stripped.startswith("[[")
            or stripped in {
                "virtual",
                "static",
                "constexpr",
                "explicit",
            }
            or stripped.endswith(
                (
                    "IoResult",
                    "std::string",
                    "std::size_t",
                    "std::uint16_t",
                    "std::uint32_t",
                    "bool",
                    "void",
                )
            )
        ):
            index -= 1
            inspected += 1
            continue
        break

    if index < 0 or not lines[index].strip().endswith("*/"):
        return ""

    end = index
    while index >= 0 and "/**" not in lines[index]:
        index -= 1

    if index < 0:
        return ""

    return "\n".join(lines[index : end + 1])


def declarations(path: Path) -> list[Declaration]:
    """Collect documented function declarations from one public header."""
    lines = path.read_text(encoding="utf-8").splitlines()
    result: list[Declaration] = []
    index = 0

    while index < len(lines):
        stripped = lines[index].strip()

        if (
            "(" not in stripped
            or stripped.startswith(
                (
                    "#",
                    "//",
                    "*",
                    "if ",
                    "for ",
                    "while ",
                    "switch ",
                    "return ",
                    "static_assert",
                )
            )
        ):
            index += 1
            continue

        start = index
        collected = [lines[index]]

        while (
            ";" not in " ".join(collected)
            and index + 1 < len(lines)
            and len(collected) < 16
        ):
            index += 1
            collected.append(lines[index])

        collapsed = " ".join(part.strip() for part in collected)

        prefix = collapsed.split("(", 1)[0]
        if "." in prefix or "->" in prefix or prefix.rstrip().endswith("["):
            index += 1
            continue

        if (
            collapsed.endswith(";")
            and ")" in collapsed
            and "{" not in collapsed
            and not collapsed.startswith("using ")
            and "=" not in prefix
            and not prefix.startswith(("const auto", "auto "))
            and not any(
                token in collapsed
                for token in (
                    "static_cast<",
                    "digitalWrite(",
                    "pinMode(",
                    "EEPROM.",
                    "wire_.",
                    "spi_.",
                    "writeHigh(",
                    "applyDirection(",
                    "analogWrite(",
                    "va_start(",
                    "va_end(",
                )
            )
        ):
            result.append(
                Declaration(
                    path=path,
                    line=start + 1,
                    text=collapsed,
                    documentation=preceding_doc(lines, start),
                )
            )

        index += 1

    return result


def audit_declaration(declaration: Declaration) -> list[str]:
    """Return documentation failures for one declaration."""
    failures: list[str] = []
    documentation = declaration.documentation

    if not documentation:
        return ["missing Doxygen block"]

    if "@brief" not in documentation:
        failures.append("missing @brief")

    before_parameters = declaration.text.split("(", 1)[0].strip()
    name_token = before_parameters.split()[-1]
    function_name = name_token.split("::")[-1]
    return_type = before_parameters[: -len(name_token)].strip()
    parameter_text = declaration.text.split("(", 1)[1].rsplit(")", 1)[0]

    for parameter in split_parameters(parameter_text):
        name = parameter_name(parameter)
        if name is not None and not re.search(
            rf"@param(?:\[[^\]]+\])?\s+{re.escape(name)}\b",
            documentation,
        ):
            failures.append(f"missing @param {name}")

    constructor_or_destructor = (
        not return_type
        or return_type in {"explicit", "constexpr", "constexpr explicit"}
        or function_name.startswith("~")
        or function_name == "operator="
    )
    returns_void = return_type.endswith("void")

    if (
        not constructor_or_destructor
        and not returns_void
        and "@return" not in documentation
    ):
        failures.append("missing @return")

    return failures


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    failures: list[str] = []

    for path in sorted((root / "libraries").rglob("*.hpp")):
        text = path.read_text(encoding="utf-8")

        if "@file" not in text or "@brief" not in text:
            failures.append(f"{path}: missing file-level @file or @brief")

        for declaration in declarations(path):
            for failure in audit_declaration(declaration):
                failures.append(
                    f"{declaration.path}:{declaration.line}: "
                    f"{failure}: {declaration.text}"
                )

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1

    print("Public-header Doxygen audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
