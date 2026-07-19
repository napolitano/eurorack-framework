# Getting Started

## 1. Install the development tools

Required:

- Git
- Python 3.10 or newer
- PlatformIO Core
- GCC or Clang with C++17 support
- Doxygen
- Graphviz
- a LaTeX distribution when PDF output is required
- clang-format
- cppcheck

Typical packages on Ubuntu or Debian:

```bash
sudo apt-get update
sudo apt-get install \
    build-essential clang clang-format cppcheck \
    doxygen graphviz texlive-latex-extra texlive-fonts-recommended
```

On Windows, install PlatformIO, LLVM or a supported native compiler, Doxygen,
Graphviz, and MiKTeX or TeX Live. Add each executable directory to `PATH`.

## 2. Create a Python environment

```bash
python -m venv .venv
```

Linux and macOS:

```bash
source .venv/bin/activate
```

Windows PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Install PlatformIO:

```bash
python -m pip install --upgrade pip platformio
```

## 3. Run the tests

PlatformIO:

```bash
pio test -e native
```

Strict standalone builds:

```bash
python tools/run-native-tests.py --compiler g++
python tools/run-native-tests.py --compiler clang++
```

Documentation audits:

```bash
python tools/check-public-docs.py
python tools/check-maintainability-docs.py
```

## 4. Build the documentation

Generate the Doxygen LaTeX tree and PDF:

```bash
python tools/build-documentation.py
```

Generated outputs:

```text
docs/generated/latex/
dist/eurorack-framework-<version>-manual.pdf
dist/eurorack-framework-<version>-markdown-docs.zip
```

The Markdown documentation remains usable without Doxygen:

```text
docs/manual/
docs/guides/
docs/architecture/
docs/release/
```

## 5. Include the framework

Public headers live below:

```cpp
#include <eurorack/...>
```

Do not include files from `src/` directly. Application code should depend on the
public interfaces and instantiate platform adapters at the hardware boundary.

## First design rule

Microcontroller pins are not Eurorack interfaces. Always provide appropriate
input protection, attenuation, level shifting, output buffering, current
limiting, and calibration in hardware.
