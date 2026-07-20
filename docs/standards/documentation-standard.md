# Documentation standard

This document defines the mandatory documentation model for the Eurorack Framework.

## Documentation layers

- `docs/architecture/`: cross-cutting design decisions and resource ownership.
- `docs/hardware/`: device-oriented explanations for supported ICs and IC families.
- `docs/libraries/`: software-oriented guides for public libraries.
- `docs/guides/`: task-oriented integration and development instructions.
- `docs/manual/`: framework-level user manual.
- `docs/reference/`: generated or consolidated reference material.

## Hardware documentation contract

Every concrete IC driver library shall provide `hardware-doc.json` and one dedicated page below `docs/hardware/`. The metadata shall validate against `schemas/hardware-doc.schema.json`.

A hardware page shall contain these sections:

1. Overview
2. Hardware function
3. Key electrical and protocol characteristics
4. Pin and signal integration
5. Framework operation sequence
6. Framework support
7. Dependencies and ownership
8. Minimal code example
9. Integration notes
10. Example and tests
11. Manufacturer references

The page shall distinguish hardware behavior from framework behavior, identify unsupported features, describe resource ownership and lifetime, and link to primary manufacturer material. Code fragments are explanatory; executable examples remain the authoritative source.

## Library documentation contract

Every concrete IC driver library shall provide a software-oriented page below `docs/libraries/`. Hardware pages do not replace library pages. Library pages shall document purpose, public API, lifecycle and configuration, ownership and memory, error handling, ISR/reentrancy/concurrency, dependencies and resources, executable usage, tests, limitations, and related documentation. Concrete driver metadata shall reference the page through `libraryDocumentation`.

## Executable examples

Metadata example entries shall be repository-relative directories containing `main.cpp`. Each example shall include the documented library's public header and shall be compiled by `tools/build-examples.py` when it resides below `examples/components/`. Include-only placeholders are forbidden. A driver example shall construct the driver, perform at least one representative operation, inspect or propagate the result, and provide a README explaining which adapters must be replaced on target hardware.

## Test mappings

Metadata test entries shall be repository-relative files or directories. They identify the suites that exercise the driver. A mapping does not by itself claim complete coverage.

## References

Product pages, data sheets, errata, and application notes shall use primary manufacturer sources where available. External reference validation is a separate network-enabled quality gate.

## Status

The accepted maturity values are `experimental`, `alpha`, `beta`, `stable`, and `deprecated`. The framework currently uses alpha status for supported drivers unless explicitly stated otherwise.
