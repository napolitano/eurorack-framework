# Continuous Integration

The repository separates quality checks, tests, embedded builds, documentation,
and manual development artifacts into distinct GitHub Actions workflows.

This separation makes the README badges meaningful. A green test badge means
the test workflow passed; it is not merely a different job hidden inside a
larger workflow.

## Quality

`.github/workflows/ci.yml`

Checks:

- public API documentation
- maintainability documentation
- Markdown documentation packaging
- licensing consistency and SPDX metadata
- clang-format
- cppcheck

README badge label: `Quality`

## Tests

`.github/workflows/tests.yml`

Checks:

- strict GCC native tests
- strict Clang native tests
- AddressSanitizer and UndefinedBehaviorSanitizer
- all native component examples

README badge label: `Tests`

## Builds

`.github/workflows/build.yml`

Checks:

- PlatformIO native integration
- pure attenuverter model
- Arduino Nano R3 attenuverter compilation

README badge label: `Builds`

## Documentation

`.github/workflows/documentation.yml`

Builds:

- Markdown documentation archive
- combined Doxygen and LaTeX PDF manual

README badge label: `Documentation`

## Development artifacts

`.github/workflows/artifacts.yml` remains manual because the project is
Unreleased Alpha. It uploads temporary Actions artifacts and does not create a
Git tag, GitHub Release, or package-registry publication.

## License-sensitive changes

`.github/CODEOWNERS` assigns licensing files, package metadata, and workflow
definitions to the repository owner. Branch protection should require owner
review for CODEOWNERS changes.

`tools/check-licensing.py` verifies:

- required license documents exist
- root and SPDX/REUSE license copies are identical
- `library.json` carries the correct SPDX identifier
- framework C and C++ files contain the expected SPDX identifier
- NOTICE includes the required notice lines

## Recommended branch protection

Protect `main` and require:

- Quality
- Tests
- Builds
- Documentation Build

Also require pull requests, dismiss stale approvals after new commits, and
require CODEOWNER review. Do not allow ordinary merges to bypass failed checks.

## Badge behavior

GitHub workflow badges show the state of the named workflow on `main`. During
initial setup they may display an unknown state until the workflow has run at
least once on the default branch.
