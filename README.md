# Eurorack Framework

[![Quality](https://github.com/napolitano/eurorack-framework/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/napolitano/eurorack-framework/actions/workflows/ci.yml)
[![Tests](https://github.com/napolitano/eurorack-framework/actions/workflows/tests.yml/badge.svg?branch=main)](https://github.com/napolitano/eurorack-framework/actions/workflows/tests.yml)
[![Builds](https://github.com/napolitano/eurorack-framework/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/napolitano/eurorack-framework/actions/workflows/build.yml)
[![Documentation](https://github.com/napolitano/eurorack-framework/actions/workflows/documentation.yml/badge.svg?branch=main)](https://github.com/napolitano/eurorack-framework/actions/workflows/documentation.yml)
[![License](https://img.shields.io/badge/license-PolyForm%20Noncommercial%201.0.0-2f6f9f)](LICENSE)
[![Status](https://img.shields.io/badge/status-unreleased%20alpha-c97800)](docs/release/project-maturity.md)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)](https://en.cppreference.com/w/cpp/17)

Portable C++ infrastructure for reusable Eurorack module firmware.

> [!WARNING]
> **Development status - not intended for production firmware**
>
> This project is still under active development. Its public API, data formats,
> timing behavior, hardware assumptions, and driver implementations may change
> without backward compatibility before the first stable release. The framework
> has not yet completed validation on every advertised microcontroller core or
> every supported peripheral.
>
> Do not use this release in safety-critical equipment, commercial products, or
> production firmware that requires a stable and fully qualified dependency.
> Anyone who nevertheless uses the framework does so entirely at their own risk
> and remains responsible for electrical protection, calibration, regulatory
> compliance, testing, validation, maintenance, and all consequences of use.

## Project maturity

**Status: Unreleased - Alpha**

The current version number is a development identifier, not a published stable
release. The project is still establishing its API, supported platforms,
hardware validation matrix, documentation, examples, and release process.

The framework should currently be treated as experimental infrastructure.
Breaking changes are expected. Persistent formats, behavior, interfaces, and
hardware assumptions may change without migration support.

The framework provides hardware-independent controls, electrical signal models,
configuration, persistence, display rendering, peripheral drivers, Arduino
adapters, and deterministic native simulation. Concrete module behavior belongs
in consuming projects and is deliberately excluded from this repository.

Development artifact identifier: **0.1.0-alpha.10**

## Architectural boundary

This repository contains reusable infrastructure. Product-specific signal
processing, musical behavior, user interfaces, and module application logic
belong in consuming firmware projects.

```text
Consuming module firmware
        |
        v
Reusable Eurorack Framework
        |
        v
Platform adapters and peripheral drivers
        |
        v
Physical or simulated hardware
```

A consuming firmware project may use `CvInput`, `CvOutput`, `GateInput`,
buttons, LEDs, storage, display primitives, and peripheral drivers while keeping
its product-specific behavior outside the framework.

## Repository structure

```text
include/
  eurorack_config.hpp     Human-editable Eurorack defaults
  eurorack/
    controls/             Buttons, LEDs, encoders, potentiometers
    core/                 Framework configuration and foundational types
    display/              Canvas, graphics, text, widgets
    drivers/              External IC and display-controller drivers
    io/                   Generic electrical and bus abstractions
    platform/arduino/     Arduino Core adapters
    simulation/           Deterministic native test doubles and export
    storage/              Persistence, serialization, CRC, record storage

src/                      Nontrivial implementations
examples/                 Small component examples, never complete modules
tests/native/             Host-based unit tests
docs/                     Architecture, guides, reference, and release audits
tools/                    Formatting, tests, documentation, and artifact scripts
```

## Requirements

The development workflow is supported on Windows, Linux, and macOS. The exact
package names differ by operating system, but the required tools are the same:

- Git
- Python 3.10 or newer
- PlatformIO Core 6.x
- a C++17 compiler - GCC and Clang are used by CI
- Doxygen 1.9 or newer
- Graphviz for diagrams and call graphs
- clang-format
- cppcheck

Arduino development additionally requires the PlatformIO platform and board
package for the chosen target.

## Development environment setup

### 1. Clone the repository

```bash
git clone https://github.com/napolitano/eurorack-framework.git
cd eurorack-framework
```

### 2. Create a Python virtual environment

Linux and macOS:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install platformio
```

Windows PowerShell:

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install platformio
```

### 3. Install native development tools

Ubuntu or Debian:

```bash
sudo apt-get update
sudo apt-get install \
    build-essential clang clang-format cppcheck doxygen graphviz
```

macOS with Homebrew:

```bash
brew install llvm clang-format cppcheck doxygen graphviz
```

Windows options include Visual Studio Build Tools, LLVM, Doxygen, Graphviz, and
cppcheck. Add their executable directories to `PATH` before opening the
PlatformIO shell.

### 4. Verify the environment

```bash
python --version
pio --version
g++ --version
clang++ --version
doxygen --version
dot -V
clang-format --version
cppcheck --version
```

See `docs/guides/development-environment.md` for troubleshooting, recommended
editor configuration, and board-specific setup.

## Examples

Small native examples for the individual control and signal models are located
under:

```text
examples/components/
```

Build and execute all component examples:

```bash
python tools/build-examples.py
```

A complete Arduino Nano R3 example application is located at:

```text
examples/projects/nano-r3-attenuverter/
```

It implements a slow one-channel CV attenuverter with a protected bipolar input,
a panel gain control, an MCP4922 DAC, and a bipolar op-amp output stage. The
example documentation explicitly defines the required analog transfer
functions, calibration values, and electrical safety limits.

## Continuous integration

GitHub Actions validates every pull request and relevant push with GCC, Clang,
sanitizers, PlatformIO, documentation audits, formatting, and cppcheck.

Documentation changes additionally build the Markdown archive and combined PDF
manual. A separate manual workflow creates ephemeral Unreleased Alpha artifacts
without publishing a GitHub Release.

See `docs/guides/continuous-integration.md`.

## Building and testing

### PlatformIO native tests

```bash
pio test -e native
```

The framework warning policy applies only to project sources through
`build_src_flags`. PlatformIO-provided Unity C sources are not incorrectly
forced through the framework's C++ warning profile.

Clean stale PlatformIO output after changing compiler flags or test framework
configuration:

```bash
pio run --target clean
```

Or remove the complete `.pio` directory.

### Standalone native test runner

The repository also includes a PlatformIO-independent runner:

```bash
python tools/run-native-tests.py --compiler g++
python tools/run-native-tests.py --compiler clang++
python tools/run-native-tests.py --compiler g++ --sanitizers
```

The sanitizer mode enables AddressSanitizer and UndefinedBehaviorSanitizer.

### Formatting

Windows PowerShell:

```powershell
.\tools\format.ps1
.\tools\check-format.ps1
```

Cross-platform direct invocation:

```bash
find include src tests examples -type f \
    \( -name '*.hpp' -o -name '*.cpp' \) -print0 \
    | xargs -0 clang-format -i
```

### Static analysis

```bash
cppcheck \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    -I include \
    src
```

## Documentation

The primary documentation is maintained as Markdown:

```text
docs/manual/
docs/guides/
docs/architecture/
docs/release/
```

Start with:

```text
docs/manual/index.md
```

Doxygen consumes the Markdown manual and the documented C++ API, then generates
a LaTeX tree for PDF output. HTML generation is intentionally disabled because
the project documentation is meant to remain useful directly in the repository.

Build Markdown and PDF documentation:

```bash
python tools/build-documentation.py
```

Build only the Markdown documentation archive:

```bash
python tools/build-documentation.py --markdown-only
```

A complete PDF build requires Doxygen, Graphviz, `make`, and a LaTeX
distribution such as TeX Live or MiKTeX.

Generated release artifacts:

```text
dist/
  eurorack-framework-<version>-markdown-docs.zip
  eurorack-framework-<version>-manual.pdf
```

Doxygen does not provide a native Markdown-output generator. Markdown is
therefore treated as the maintained source and publication format, while
Doxygen supplies cross-referenced API content and optional PDF generation.

## Generating release artifacts

The cross-platform artifact script performs validation and creates distributable
files:

```bash
python tools/build-artifacts.py
```

By default it creates:

```text
dist/
  eurorack-framework-<version>-source.zip
  eurorack-framework-<version>-documentation.zip
  eurorack-framework-<version>-checksums.sha256
```

The source archive excludes local build caches, generated documentation,
editor metadata, and temporary files. The documentation artifacts contain the maintained Markdown manual and, when LaTeX is available, a combined PDF manual and API reference.

Available switches:

```bash
python tools/build-artifacts.py --help
python tools/build-artifacts.py --skip-tests
python tools/build-artifacts.py --skip-documentation
python tools/build-artifacts.py --output-directory out
```

A normal release build should not use either skip option. Detailed artifact
contents and the release procedure are documented in
`docs/guides/building-artifacts.md`.

## Human-editable configuration

Framework-wide assumptions are stored in:

```text
include/eurorack_config.hpp
```

The file contains readable constants for audio, CV, pitch, gates, triggers,
interaction timing, display timing, and simulation timing. A consuming project
may replace it by defining `EURORACK_FRAMEWORK_CONFIG_FILE` to the path of an
alternative header.

Every configuration value affects firmware behavior and must be reviewed for
the target hardware. Defaults are not a substitute for electrical design,
measurement, or calibration.

## Major components

The public API currently includes:

- momentary buttons, digital LEDs, bicolor LEDs, and rotary encoders
- potentiometer, analog input, CV, gate, and trigger models
- SPI, I2C, digital I/O, analog I/O, and time interfaces
- MCP4922 and DAC8568 DAC drivers
- MCP23017 GPIO expansion
- TLC5916 and TLC5947 LED drivers
- 74HC165 and 74HC595 shift-register drivers
- SSD1306 and SH1106 monochrome display drivers
- canvas primitives, text, glyphs, and reusable UI widgets
- explicit byte encoding, CRC-32, dual-slot records, and storage backends
- Arduino Core adapters
- native simulation, virtual buses, scenarios, and canvas export

Each public declaration is documented in the generated Doxygen reference.
Architecture-level explanations are in `docs/architecture`.

## Platform and electrical responsibility

Public APIs remain independent of Arduino-specific types. Arduino adapters are
provided as one concrete platform layer, not as an electrical interface to a
Eurorack system.

Microcontroller pins must never be connected directly to Eurorack jacks merely
because an adapter exposes a digital or analog method. Input protection,
attenuation, clamping, impedance, filtering, level shifting, output buffering,
short-circuit protection, power integrity, and calibration remain responsibilities
of the hardware and firmware integrator.

## API and real-time expectations

The generic control and I/O layers avoid hidden ownership and generally operate
synchronously. Referenced buses, pins, clocks, and storage backends must outlive
the objects that consume them.

The framework does not claim that every operation is safe inside an interrupt
service routine. Bus transactions, file access, dynamic allocation in native
simulation, and platform SDK calls may block. Each API's Doxygen documentation
states the relevant ownership, allocation, and execution assumptions.

## Documentation map

- `docs/guides/development-environment.md` - toolchain setup
- `docs/guides/building-artifacts.md` - reproducible artifacts
- `docs/guides/framework-configuration.md` - compile-time configuration
- `docs/guides/examples.md` - example structure and documentation requirements
- `docs/architecture/framework-boundary.md` - project scope
- `docs/architecture/hardware-interfaces.md` - abstraction contracts
- `docs/architecture/persistence.md` - storage and record guarantees
- `docs/architecture/display-primitives.md` - canvas representation
- `docs/architecture/simulation.md` - deterministic native simulation
- `docs/release` - release-specific audits and known limitations

## Versioning

Pre-1.0 and release-candidate versions may change API and persistent data
formats without migration support. A future stable publication release requires successful
CI, documentation, static analysis, native sanitizer tests, and builds against
the explicitly supported embedded cores.

## Maintaining and documenting the code

Documentation is treated as part of the implementation. Public API comments
must explain semantics, ownership, units, failure behavior, timing, and hardware
boundaries. Nontrivial implementation decisions require nearby comments that
explain why the code is structured that way.

The complete standard and review procedure are documented in
`docs/guides/code-maintenance-and-documentation.md`.

Run both documentation audits before committing:

```bash
python tools/check-public-docs.py
python tools/check-maintainability-docs.py
```

## Extended panel controls

The control layer includes illuminated pushbuttons, two-position toggle
switches, DIP-switch banks, On-Off-(On) switches, RGB LEDs, linear faders, and
illuminated faders. See `docs/architecture/extended-panel-controls.md`.

## Licensing

This repository is source-available software.

- `LICENSE` contains PolyForm Noncommercial License 1.0.0.
- `ADDITIONAL_PERMISSION.md` grants limited five-unit cost-recovery permission.
- `COMMERCIAL_LICENSE.md` explains when written authorization is required.
- `NOTICE` contains attribution and canonical project information.

This is not an OSI-approved open-source license because commercial use is
restricted.

## Author

**Axel Napolitano**  
Email: eurorack@skjt.de  
Canonical repository: https://github.com/napolitano/eurorack-framework
