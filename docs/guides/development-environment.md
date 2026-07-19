# Development Environment

This guide describes a reproducible development environment for native testing,
Arduino integration, documentation generation, formatting, and static analysis.

## Supported development hosts

The framework itself is portable C++17. The supplied scripts are designed for:

- Linux with GCC or Clang
- macOS with Apple Clang or Homebrew LLVM
- Windows with a PlatformIO-supported native compiler or LLVM

CI remains the authoritative reference environment. A local toolchain may emit
additional diagnostics because compiler versions differ.

## Required tools

### Python and PlatformIO

Use Python 3.10 or newer. Install PlatformIO in a project-local virtual
environment to avoid mixing framework tooling with unrelated Python packages.

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip platformio
```

On PowerShell, activate with:

```powershell
.\.venv\Scripts\Activate.ps1
```

### Native compilers

Install both GCC and Clang when possible. The codebase is compiled with C++17,
strict conversion diagnostics, and warnings as errors. Passing only one compiler
is insufficient for release qualification because the compilers diagnose
different classes of defects.

### Documentation tools

Doxygen parses Markdown and C++ API documentation and generates a LaTeX tree. Graphviz supplies class and dependency diagrams. A LaTeX distribution such as TeX Live or MiKTeX compiles the final PDF. Confirm that `doxygen`, `dot`, `make`, and `pdflatex` resolve from the same shell.

### Quality tools

`clang-format` controls source formatting. `cppcheck` performs an additional
portable static-analysis pass. These tools are independent of the compiler and
must be installed separately.

## Visual Studio Code

Recommended extensions:

- PlatformIO IDE
- C/C++ or clangd
- Doxygen Documentation Generator
- EditorConfig

Do not let multiple formatting extensions compete. Configure format-on-save to
use the repository `.clang-format` file.

## PlatformIO package installation

The native environment is installed automatically by PlatformIO when running:

```bash
pio test -e native
```

Embedded environments should be added explicitly to `platformio.ini` or to a
consuming firmware repository. Board definitions belong to the firmware project
when they are not part of the framework's release validation matrix.

## Common failures

### Unity warnings fail the build

Strict warnings must be configured through `build_src_flags`, not global
`build_flags`. Global flags also affect PlatformIO dependencies and can turn
warnings in Unity C sources into unrelated framework failures.

### Stale PlatformIO objects

After changing flags or dependencies, run:

```bash
pio run --target clean
```

If the problem persists, remove `.pio` completely.

### Doxygen cannot find Graphviz

Ensure the directory containing `dot` is on `PATH`. Verify with:

```bash
dot -V
```

### Different clang-format output

Use the clang-format major version selected by CI. Formatting can change between
major releases even when `.clang-format` remains unchanged.

## PlatformIO Unity conversion warnings

The native PlatformIO environment keeps framework warnings strict but exempts
conversion warnings originating in PlatformIO's generated Unity C support from
`-Werror`. Unity 2.x passes `char` and `unsigned int` values through its output
callback and can otherwise fail under `-Wsign-conversion`.

This exemption does not replace the strict framework build. Run:

```bash
python tools/run-native-tests.py --compiler g++
python tools/run-native-tests.py --compiler clang++
```

Those commands compile the framework sources with `-Wconversion`,
`-Wsign-conversion`, and `-Werror`.

After changing `platformio.ini`, remove the old build cache:

```bash
pio run --target clean
pio test -e native
```

## Optional PlatformIO board templates

The root `platformio.ini` keeps only the native environment active. It also
contains fully documented, commented templates for:

- Arduino Nano R3 with classic bootloader
- Arduino Nano R3 with new bootloader
- Arduino Nano R4
- Arduino Nano ESP32
- STM32 Nucleo F411RE
- common STM32F407VET6 Black development boards
- Raspberry Pi Pico
- Arduino Nano RP2040 Connect
- Arduino Nano 33 IoT

These blocks are examples, not a validation claim. Activate only one complete
block at a time and verify the board ID, upload method, core, and actual hardware.

A successful embedded compilation does not prove electrical compatibility,
driver behavior, timing, or analog accuracy.

