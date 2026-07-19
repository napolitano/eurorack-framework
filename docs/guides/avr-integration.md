# AVR Integration Guide

## Why this exists

Earlier framework versions shipped one monolithic library at the repository root: adding it via
PlatformIO pulled in every subsystem — including desktop-only simulation, file-backed storage,
and the full display stack — regardless of what a given firmware actually used. On an
ATmega328P, with roughly 2 KiB of SRAM and 30 KiB of usable flash, that is not workable, and the
practical workaround (including selected framework `.cpp` files directly from an application's
`main.cpp` translation unit) is not maintainable and defeats normal dependency tracking.

The framework is now split into a set of independent PlatformIO libraries under `libraries/`. A
consuming project selects only the ones it needs; PlatformIO's Library Dependency Finder compiles
and links exactly that set, nothing more.

## Why this is a local-checkout pattern, not a plain remote dependency

PlatformIO does not reliably support picking individual libraries out of a single repository when
that repository is referenced as one remote `lib_deps` entry (e.g. a bare GitHub URL): it resolves
at most one library per such entry, using whichever `library.json` sits at the reference root.
Splitting into per-component libraries only becomes selectable once the repository is available
as a local directory tree, via `lib_extra_dirs`. In practice this means:

- **Git submodule or plain clone (recommended):** check out this repository into your project
  (for example as a git submodule under `libs/eurorack-framework/`), then point
  `lib_extra_dirs` at its `libraries/` folder. This is the supported, verified path and the one
  demonstrated by both reference examples.
- **Bare remote `lib_deps` URL, no local checkout:** not supported for selective compilation.
  PlatformIO will only see whichever manifest is at the repository root, which no longer describes
  a buildable library (see `library.json` at the repository root).

## Available libraries

| Library | Contents | Depends on |
|---|---|---|
| `eurorack-compat` | AVR standard-library shims (`<cstdint>`, `<array>`, `<algorithm>`, ...), selected automatically via `__AVR__` | — |
| `eurorack-core` | Electrical/timing default configuration, `eurorack_config.hpp` override point | compat |
| `eurorack-io` | Abstract I/O interfaces (analog, digital, SPI, I2C, time), jack and calibration models | compat, core |
| `eurorack-drivers-led-interface` | Abstract `LedChannel`/`LedBank` interfaces | compat, io |
| `eurorack-controls` | Panel-control state models (buttons, encoders, faders, LEDs, switches) | compat, core, drivers-led-interface |
| `eurorack-display` | Monochrome canvas, drawing primitives, bitmap-font text, UI widgets | compat |
| `eurorack-drivers-dac` | MCP4922, DAC8568 | compat, io |
| `eurorack-drivers-led` | TLC5916, TLC5947 | compat, io, drivers-led-interface |
| `eurorack-drivers-gpio` | MCP23017 | compat, io |
| `eurorack-drivers-shift` | 74HC165, 74HC595 | compat, io |
| `eurorack-drivers-mux` | Binary-addressed analog multiplexer (74HC4051/4067-family), header-only | compat, io |
| `eurorack-drivers-display` | SSD1306, SH1106 | compat, io, display |
| `eurorack-storage` | In-memory and file-backed persistent storage, `RecordStore` | compat, io |
| `eurorack-simulation` | Native-only virtual hardware backends for host-side testing | compat, io, display |
| `eurorack-platform-arduino` | Arduino-core adapters implementing the io interfaces (GPIO, analog, SPI, I2C, time, EEPROM) | compat, io, storage; requires `framework = arduino` |

Public include paths are unchanged (`#include <eurorack/controls/rotary_encoder.hpp>` etc.);
only each header's physical location moved.

## Minimal `platformio.ini` pattern

```ini
[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328
framework = arduino
build_unflags = -std=gnu++11
build_flags = -std=gnu++17 -Wall -Wextra -Wpedantic

lib_extra_dirs = <path-to-checkout>/libraries
lib_deps =
    eurorack-compat
    eurorack-core
    eurorack-io
    eurorack-drivers-led-interface
    eurorack-controls
    eurorack-drivers-dac
```

List only the libraries your firmware actually uses; each library's own `library.json`
`dependencies` field pulls in the rest of its own requirement chain automatically.

See `examples/projects/nano-r3-attenuverter/platformio.ini` (SPI/DAC only) and
`examples/projects/nano-r3-oled-controller/platformio.ini` (I2C/OLED/GPIO-expander/mux) for two
complete, independently build-verified references.

## Known toolchain caveat

Arduino AVR's own bundled `SPI.h` (`SPIClass::transfer16`) uses an anonymous struct inside a
union, which `-Wpedantic` correctly flags as a non-standard extension. This is vendored
Arduino-core code, not framework code, and cannot be fixed from here. Projects that both use SPI
and want `-Werror` need `-Wno-error=pedantic` (keeping `-Wpedantic` itself enabled as a
diagnostic); see `nano-r3-attenuverter`'s `platformio.ini` for the exact pattern. Projects that
only use I2C (no `<SPI.h>`, like `nano-r3-oled-controller`) do not hit this and can use plain
`-Werror`.

## Native tests

`platformio.ini` at the repository root and `tools/run-native-tests.py` (a PlatformIO-independent
runner) both reference `libraries/*/include` and `libraries/*/src` directly and exercise all
libraries except `eurorack-platform-arduino` (which requires the Arduino framework headers).
