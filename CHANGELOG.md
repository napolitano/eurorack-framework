# Changelog

## 0.1.0-alpha.27

### Changed

- Updated GitHub Actions to use explicit runners, parallel per-driver sanitizer jobs, and corrected static-analysis include discovery.
- Added a dependency-free workflow policy check and integrated it into release preflight.
- Reformatted minified component examples and native driver adapters as readable reference code.
- Added an examples guide and a GitHub Actions/local CI guide; linked local validation from contributing documentation.

## [0.1.0-alpha.27] - 2026-07-20

### Added

- Added portable source-hygiene and formatting gates.
- Added strict host static analysis with public-header self-containment checks and optional `clang-tidy`/`cppcheck` integration.
- Added deterministic source-archive content validation.
- Added dedicated driver-suite selection for sanitizer runs.
- Added the quality-tooling guide.

### Changed

- Release preflight now runs source hygiene and portable formatting by default.
- Release preflight accepts explicit `--static-analysis`, `--sanitizers`, `--coverage`, and `--archive` gates.
- Artifact creation validates the generated source ZIP before publishing checksums.

## [0.1.0-alpha.25] - 2026-07-20

### Added

- Added reproducible GCC/gcov line and branch coverage reporting.
- Added general and driver-specific coverage thresholds in `tools/coverage-policy.json`.
- Added focused coverage execution by suite name and dedicated-driver suite selection.
- Added machine-readable JSON and human-readable Markdown coverage reports.
- Added a native coverage guide.

### Changed

- Coverage now resolves the same granular library dependency closure as the native test runner.

## [0.1.0-alpha.25] - 2026-07-20

### Added

- Added one dedicated native protocol suite for each of the twelve concrete IC drivers.
- Added an automatically generated driver test matrix under `docs/architecture/driver-test-matrix.md`.
- Added `--filter` support to the native test runner for focused suite execution.

### Changed

- Hardware metadata now maps every concrete driver to exactly one dedicated native suite.
- Hardware documentation validation now verifies test directory existence, public-header inclusion, dedicated naming, and at least two test cases.

All notable changes to Eurorack Framework are documented in this file. The format follows Keep a Changelog. Semantic-version compatibility is not guaranteed while the project remains Unreleased Alpha.

## [0.1.0-alpha.25] - 2026-07-20

### Added

- Functional native examples for all twelve concrete IC driver libraries.
- Shared in-memory example adapters for SPI, I2C, GPIO, analog input, and delays.
- Device-specific electrical, signal-integration, protocol, and operation-sequence sections in every IC guide.

### Changed

- Hardware documentation validation now rejects include-only examples, missing example READMEs, and examples without explicit result handling.
- The example builder now accounts for shared example-support headers when resolving granular dependencies.
- The documentation standard now requires device characteristics, signal mapping, and a framework operation sequence.

## [Unreleased]

Development artifact identifier: `0.1.0-alpha.25`.

### Added

- Added a dedicated hardware documentation page for every concrete supported IC or IC family.
- Added per-driver `hardware-doc.json` metadata linking library, examples, documentation and manufacturer references.
- Added `tools/check-hardware-docs.py` and integrated it into the release preflight.
- Added a central supported-hardware index covering DAC, ADC, GPIO, shift-register, LED, display and multiplexer drivers.

- Added an automatically generated Mermaid dependency map and machine-readable AVR resource registry.
- Added resource-conflict, documentation-coverage, and release-preflight tooling.
- Added generic allocation-free event queue, press classifier, soft-takeover, and encoder-acceleration libraries.
- Added independently selectable MCP4822 DAC and MCP3208 ADC drivers.
- Added dedicated policy and SPI converter protocol test suites.
- Added framework-level testing and resource-ownership guides.

### Changed

- Expanded framework quality checks without introducing application-specific behavior.
- Updated all manifests and root metadata to `0.1.0-alpha.25`.

## [0.1.0-alpha.14] - 2026-07-18

### Added

- Added platform-neutral controls, CV, gate, trigger, jack, display, persistence, simulation, and
  hardware-interface models.
- Added MCP4922, DAC8568, MCP23017, TLC5916, TLC5947, 74HC165, 74HC595, SSD1306, and SH1106
  drivers.
- Added Arduino Core adapters and native virtual hardware backends.
- Added native GCC, Clang, sanitizer, PlatformIO, documentation, formatting, licensing, and static
  analysis validation.
- Added documented Arduino Nano R3 attenuverter and OLED-controller examples.
- Added PolyForm Noncommercial licensing, the Five-Unit Cost-Recovery Permission, and release
  artifact tooling.

### Changed

- Established the explicit framework boundary: reusable infrastructure belongs here; concrete module
  applications remain in consuming repositories.
- Standardized project maturity on `Unreleased - Alpha` and development identifiers in the
  `0.1.0-alpha.x` range.
- Expanded Doxygen contracts, architecture documentation, examples, and project guides.

### Fixed

- Corrected DAC8568 frame handling and display-driver tests.
- Removed dynamic allocation from SSD1306 and SH1106 framebuffer transfers.
- Isolated native-only simulation and file-storage sources from Arduino builds.
- Corrected warning handling for PlatformIO's generated Unity C sources without weakening strict
  framework C++ builds.

## 0.1.0-alpha.25

### Added
- Binding documentation standard in `docs/standards/documentation-standard.md`.
- JSON Schema for concrete IC-driver metadata.
- Dedicated, build-discovered component example directories for all documented IC drivers.
- Explicit public-header and test-suite mappings in every `hardware-doc.json`.

### Changed
- Hardware documentation validation now rejects nonexistent examples, missing `main.cpp` files, examples that omit the documented public header, nonexistent test mappings, unknown metadata fields, and invalid categories or maturity values.
