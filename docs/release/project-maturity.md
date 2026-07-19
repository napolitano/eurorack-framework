# Project Maturity

## Current status

**Unreleased - Alpha**

The framework is an active development project. The package version exists only
to distinguish development artifacts and does not represent a published
release.

No stability promise is made for:

- public APIs
- persistent data formats
- timing behavior
- supported boards
- peripheral drivers
- documentation structure
- application examples
- hardware assumptions

Breaking changes remain expected.

## Why Alpha is appropriate

The project is still establishing several fundamentals:

- planned public APIs are incomplete
- representative examples are still being added
- embedded platform coverage is incomplete
- real hardware validation is incomplete
- performance and memory behavior are not fully qualified
- persistence compatibility is not frozen
- documentation and publication tooling are still evolving
- architectural changes remain possible

## Criteria for a later maturity increase

A later maturity increase should require evidence rather than optimistic naming:

- substantially complete planned API surface
- representative examples for every major subsystem
- complete example applications
- repeatable builds on declared embedded targets
- real hardware validation of major drivers
- documented timing and memory characteristics
- stable persistence behavior
- completed static analysis and sanitizer runs
- publication-quality documentation
- no known architectural redesign in progress
- no known blocking defects

Until those conditions are met, the project remains Unreleased Alpha.
