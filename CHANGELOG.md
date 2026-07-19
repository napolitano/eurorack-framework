## 0.1.0-alpha.7

### Fixed

- Exempted PlatformIO-generated Unity C conversion warnings from `-Werror` in the native test environment while retaining strict standalone framework builds.
- Scoped strict compiler warnings to project source files with PlatformIO `build_src_flags`.
- Prevented `-Werror` and `-Wsign-conversion` from being applied to PlatformIO Unity C sources.

# Changelog

## 0.1.0-alpha.7 - Documentation expansion

### Added

- Expanded the Nano R3 project into an attenuverter with independent positive/negative balance.
- Added center-dead-zone processing for attenuation and balance controls.
- Added bipolar red/green output indication with magnitude-based PWM brightness.
- Separated application arithmetic into a platform-independent model with native tests.
- Reframed all analog circuitry as a hypothetical hardware contract rather than a buildable design.
- Expanded every component example with a detailed learning-oriented README.
- Added file-level and inline explanations to all component example programs.
- Added an example documentation standard and isolated temporary example builds.
- Added attenuverter firmware walkthrough, calibration, design-decision, and troubleshooting guides.
- Added 16 minimal, compilable native component examples.
- Added a reusable example build-and-run tool.
- Added an Arduino Nano R3 one-channel CV attenuverter project using MCP4922.
- Added detailed attenuverter hardware, safety, transfer-function, and calibration documentation.
- Added CI jobs for component examples and the Nano R3 project.
- Reworked GitHub Actions into separate CI, documentation, and manual artifact workflows.
- Added GCC, Clang, sanitizer, PlatformIO, documentation, formatting, and cppcheck jobs.
- Added weekly Dependabot updates for GitHub Actions.
- Added documented branch-protection recommendations.
- Replaced generated HTML documentation with a Markdown-first manual and optional Doxygen LaTeX/PDF output.
- Added newcomer-oriented manual chapters and a dedicated documentation build script.
- Updated release artifacts to package Markdown documentation and an optional combined PDF manual.
- Reworked API documentation to describe semantics, ownership, units, state transitions, failure behavior, and hardware boundaries.
- Added implementation-level rationale for storage, display conversion, simulation, controls, and bus drivers.
- Added a maintainability documentation standard and automated placeholder/density audit.
- Added illuminated RGB pushbutton model.
- Added debounced toggle switch and DIP-switch bank models.
- Added validated On-Off-(On) switch model.
- Added logical 16-bit RGB LED model.
- Added linear fader and illuminated fader models.
- Added native tests and architecture documentation for extended controls.
- Added a prominent development-status and use-at-own-risk warning to the README.
- Added complete development-environment setup instructions for Windows, Linux, and macOS.
- Added reproducible source, documentation, and checksum artifact generation.
- Added Doxygen module groups and expanded ownership, lifetime, allocation, error, electrical, and concurrency documentation throughout public headers.
- Added detailed documentation for previously sparse drivers, simulation APIs, widgets, jack models, byte codecs, and internal rendering helpers.
- Added a stricter public-header documentation audit covering briefs, named parameters, and return values.

### Changed

- Removed obsolete documents and wording inherited from the abandoned 1.x and RC naming.
- Standardized project maturity language on `Unreleased - Alpha`.
- Clarified that package versions are development artifact identifiers rather than published releases.
- Removed obsolete historical references to an uncommitted application prototype.
- Generalized framework-boundary documentation to describe consuming firmware without naming discarded implementation experiments.
- Reclassified project maturity from advanced prerelease to Unreleased Alpha.
- Reset the development version to `0.1.0-alpha.7`.
- Clarified that `1.0.0` remains a future milestone requiring substantially more validation.
- Updated Doxygen configuration and project version to 0.1.0-alpha.7.
- Reorganized README build, test, documentation, artifact, and platform-responsibility guidance.

## 0.1.0-alpha.7 - 2026-07-18

### Fixed

- Exempted PlatformIO-generated Unity C conversion warnings from `-Werror` in the native test environment while retaining strict standalone framework builds.
- Corrected DAC8568 normal data-frame expectations and special-command bit placement.
- Replaced misleading DAC8568 clear-output operation with explicit clear-code configuration.
- Corrected stale display, widget, I2C, and DAC tests uncovered by real test execution.
- Isolated native-only simulation and file-storage sources from Arduino builds.
- Removed dynamic allocation from SSD1306 and SH1106 framebuffer transfer paths.

### Added

- Reworked API documentation to describe semantics, ownership, units, state transitions, failure behavior, and hardware boundaries.
- Added implementation-level rationale for storage, display conversion, simulation, controls, and bus drivers.
- Added a maintainability documentation standard and automated placeholder/density audit.
- Added illuminated RGB pushbutton model.
- Added debounced toggle switch and DIP-switch bank models.
- Added validated On-Off-(On) switch model.
- Added logical 16-bit RGB LED model.
- Added linear fader and illuminated fader models.
- Added native tests and architecture documentation for extended controls.
- Standalone native test runner with a local Unity-compatible harness.
- GCC, Clang, AddressSanitizer, and UndefinedBehaviorSanitizer validation.
- Public-header Doxygen audit.
- GitHub Actions build, sanitizer, formatting, documentation, and static-analysis jobs.
- Release-candidate audit documentation.

All notable changes to Eurorack Framework will be documented in this file.

The format is based on Keep a Changelog. The project intends to use semantic
versioning once the public API stabilizes.

## [Unreleased]

### Changed

- Defined the repository boundary explicitly: concrete module applications live
  in consuming projects.
  gate, trigger, and encoder components.
- Added placeholder `io` and `simulation` areas to the documented framework
  structure.
- Applied repository-wide clang-format rules and warning-as-error native builds.
- Split nontrivial implementations from public headers where appropriate.

### Added

- Reworked API documentation to describe semantics, ownership, units, state transitions, failure behavior, and hardware boundaries.
- Added implementation-level rationale for storage, display conversion, simulation, controls, and bus drivers.
- Added a maintainability documentation standard and automated placeholder/density audit.
- Added illuminated RGB pushbutton model.
- Added debounced toggle switch and DIP-switch bank models.
- Added validated On-Off-(On) switch model.
- Added logical 16-bit RGB LED model.
- Added linear fader and illuminated fader models.
- Added native tests and architecture documentation for extended controls.
- Added atomically committed native `FileStorage`.
- Added Arduino EEPROM persistent-storage adapter.
- Added EEPROM region partitioning and update-based writes.
- Added native file-storage tests and storage-backend documentation.
- Added affine raw-code and voltage calibration.
- Added two-point calibration derivation.
- Added DAC8568 eight-channel 16-bit SPI DAC driver.
- Added SH1106 I2C monochrome OLED driver.
- Added native tests and architecture documentation for step 12.
- Added Arduino GPIO adapters.
- Added Arduino ADC and `analogWrite` adapters.
- Added Arduino time and delay adapter.
- Added Arduino SPI and I2C adapters.
- Added Arduino platform example and documentation.
- Added deterministic virtual time and delay.
- Added virtual digital and analog hardware channels.
- Added inspectable virtual SPI and I2C buses.
- Added response queues and error injection.
- Added deterministic simulation scenarios.
- Added PBM and ASCII canvas export.
- Added native simulation tests and architecture documentation.
- Added I2C SSD1306 display-controller driver.
- Added 128x32 and 128x64 initialization profiles.
- Added row-major canvas to SSD1306 page conversion.
- Added contrast, inversion, and display-power control.
- Added native SSD1306 tests and architecture documentation.
- Added parameter, potentiometer, and encoder widgets.
- Added progress, top-bar, and footer-bar widgets.
- Added scrollable list menus with selection visibility calculation.
- Added confirmation and error overlays.
- Added native widget tests and architecture documentation.
- Added built-in 5x7 ASCII bitmap font.
- Added text measurement and fixed-width rendering.
- Added horizontal and vertical text alignment.
- Added normal and inverted text rendering with clipping.
- Added scalable arrows, check, cross, plus, and minus glyphs.
- Added native text and glyph tests and architecture documentation.
- Added non-owning packed `MonochromeCanvas`.
- Added explicit clipping and set, clear, or invert pixel operations.
- Added horizontal, vertical, arbitrary, dashed, and dotted lines.
- Added filled and unfilled rectangles and circles.
- Added native display-primitive tests and architecture documentation.
- Added generic LED channel and bank interfaces.
- Added MCP4922 and MCP23017 drivers.
- Added TLC5916, TLC5947, 74HC595, and 74HC165 drivers.
- Added platform-neutral `PersistentStorage`.
- Added native and simulator `MemoryStorage`.
- Added explicit little-endian `ByteWriter` and `ByteReader`.
- Added CRC-32 integrity checking.
- Added versioned dual-slot atomic `RecordStore`.
- Added native persistence tests and architecture documentation.
- Added calibrated and smoothed `Potentiometer`.
- Added bounded `EncoderValue` with clamp and wrap behavior.
- Added generic analog and digital Eurorack jack models.
- Added optional connection-state modeling.
- Added native tests for step 3 models.
- Added generic digital input, output, and bidirectional pin interfaces.
- Added raw analog input and output channel interfaces.
- Added monotonic time and explicit blocking-delay interfaces.
- Added transport-neutral SPI and I2C bus interfaces.
- Added common nonexception `IoResult` status values.
- Added native contract tests using in-memory fake backends.
- Human-editable root configuration in `include/eurorack_config.hpp`.
- Compile-time project override through `EURORACK_FRAMEWORK_CONFIG_FILE`.
- Central typed and validated `FrameworkConfig`.
- Platform-independent `MomentaryButton`, `DigitalLed`, and `BiColorLed`.
- Platform-independent `RotaryEncoder`, `AnalogInput`, `CvInput`, `CvOutput`,
  `GateInput`, and `TriggerOutput`.
- Base `PersistentStorage` interface.
- Native and embedded tests for the implemented generic components.
- Strict Doxygen and contribution documentation requirements.
- Initial repository scaffold and licensing model.
