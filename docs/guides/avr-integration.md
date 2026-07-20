# AVR Integration Guide

## Scope

This guide describes how to consume Eurorack Framework from an Arduino Nano R3 or another constrained
AVR target. The central rule is simple: select the exact control, driver, interface, storage, and
platform libraries required by the firmware. Do not select a complete category unless the project
actually uses that complete category.

The ATmega328P provides approximately 2 KiB SRAM and 32 KiB flash, with less flash available when a
bootloader is present. Library boundaries do not replace linker garbage collection, but they prevent
unrelated translation units and dependency chains from entering the build in the first place.

## Supported checkout pattern

The repository contains many PlatformIO libraries below `libraries/`. PlatformIO does not reliably
select several nested libraries from a single bare remote Git dependency. Use a local clone or Git
submodule and expose its `libraries/` directory through `lib_extra_dirs`.

Recommended submodule layout:

```bash
git submodule add https://github.com/napolitano/eurorack-framework.git \
    lib/eurorack-framework
```

```ini
lib_extra_dirs = lib/eurorack-framework/libraries
```

A bare repository URL in `lib_deps` is not the supported selective-compilation path.

## Selection principles

1. Select one library for each concrete element used by the project.
2. Select one library for each concrete chip, not a complete driver category.
3. Select only the Arduino facilities used by the project.
4. Let component manifests pull their small shared contracts transitively.
5. Use compatibility meta-libraries only when compiling a complete category is intentional.
6. Keep `lib_ldf_mode = chain+` unless a documented project-specific reason requires a broader mode.
7. Verify the final ELF/map output; library selection is not a substitute for size measurement.

## Library naming

### Shared foundation

| Library | Responsibility |
|---|---|
| `eurorack-compat` | AVR standard-library compatibility shims selected through `__AVR__` |
| `eurorack-core` | Framework-wide electrical and timing configuration contract |
| `eurorack-drivers-led-interface` | Shared abstract LED channel and bank interfaces |

### Hardware-neutral I/O contracts

| Library | Responsibility |
|---|---|
| `eurorack-io-result` | Non-throwing I/O result values |
| `eurorack-io-digital` | Digital pin interfaces |
| `eurorack-io-analog` | Raw analog channel interfaces |
| `eurorack-io-spi` | SPI transaction contract |
| `eurorack-io-i2c` | I2C transfer contract |
| `eurorack-io-time` | Monotonic time and blocking-delay contracts |
| `eurorack-io-analog-calibration` | Analog calibration models |
| `eurorack-io-jack` | Analog and digital jack state models |

### Control models

| Library | Element |
|---|---|
| `eurorack-control-analog-input` | Analog input filtering and snapshots |
| `eurorack-control-bicolor-led` | Two-color LED model |
| `eurorack-control-cv` | CV input and output models |
| `eurorack-control-digital-led` | Single-color LED model |
| `eurorack-control-dip-switch` | DIP-switch bank |
| `eurorack-control-encoder-value` | Bounded encoder value |
| `eurorack-control-fader` | Linear fader |
| `eurorack-control-gate` | Gate input and trigger output |
| `eurorack-control-illuminated-button` | Illuminated button composition |
| `eurorack-control-illuminated-fader` | Illuminated fader composition |
| `eurorack-control-momentary-button` | Debounced momentary button |
| `eurorack-control-multicolor-led` | Logical RGB LED |
| `eurorack-control-on-off-momentary-switch` | On-Off-(On) switch |
| `eurorack-control-potentiometer` | Potentiometer |
| `eurorack-control-rotary-encoder` | Quadrature rotary encoder |
| `eurorack-control-toggle-switch` | Two-position toggle switch |

A module without buttons, encoders, LEDs, or faders does not need any corresponding control library.

### Concrete peripheral drivers

| Library | Device |
|---|---|
| `eurorack-driver-mcp4922` | MCP4922 dual 12-bit DAC |
| `eurorack-driver-dac8568` | DAC8568 eight-channel 16-bit DAC |
| `eurorack-driver-mcp23017` | MCP23017 GPIO expander |
| `eurorack-driver-tlc5916` | TLC5916 LED sink |
| `eurorack-driver-tlc5947` | TLC5947 PWM LED sink |
| `eurorack-driver-74hc165` | 74HC165 input shift register |
| `eurorack-driver-74hc595` | 74HC595 output shift register |
| `eurorack-driver-analog-multiplexer` | 74HC4051/74HC4067-family multiplexer |
| `eurorack-driver-ssd1306` | SSD1306 OLED controller |
| `eurorack-driver-sh1106` | SH1106 OLED controller |

A project using MCP4922 must not select DAC8568 merely because both are DACs. The same rule applies
to display controllers, LED drivers, shift registers, and every future chip driver.

### Display facilities

| Library | Responsibility |
|---|---|
| `eurorack-display-geometry` | Geometry value types |
| `eurorack-display-monochrome-canvas` | Packed framebuffer canvas |
| `eurorack-display-drawing` | Drawing primitives |
| `eurorack-display-glyphs` | Reusable glyphs |
| `eurorack-display-text` | Bitmap-font text |
| `eurorack-display-widgets` | Menus, bars, parameters, and overlays |

A display driver requires the framebuffer contract but does not automatically require text, glyphs,
or widgets.

### Arduino adapters

| Library | Arduino facility |
|---|---|
| `eurorack-platform-arduino-gpio` | Digital GPIO |
| `eurorack-platform-arduino-analog` | `analogRead` and `analogWrite` |
| `eurorack-platform-arduino-spi` | SPI |
| `eurorack-platform-arduino-i2c` | Wire/I2C |
| `eurorack-platform-arduino-time` | `millis`, `micros`, and delay |
| `eurorack-platform-arduino-eeprom` | EEPROM storage backend |

The umbrella library `eurorack-platform-arduino` intentionally selects all six facilities and should
not be used on AVR unless all six are required.

### Storage and simulation

Storage is split into `eurorack-storage-interface`, `eurorack-storage-byte-codec`,
`eurorack-storage-crc32`, `eurorack-storage-record-store`, `eurorack-storage-memory-storage`, and
`eurorack-storage-file-storage`. Native-only backends are isolated from embedded storage contracts.

Simulation is split into `eurorack-simulation-virtual-io`, `eurorack-simulation-virtual-buses`,
`eurorack-simulation-virtual-time`, `eurorack-simulation-scenario`, and
`eurorack-simulation-canvas-export`.

## Compatibility meta-libraries

The following names preserve category-level selection:

```text
eurorack-io
eurorack-controls
eurorack-display
eurorack-drivers-dac
eurorack-drivers-display
eurorack-drivers-gpio
eurorack-drivers-led
eurorack-drivers-shift
eurorack-drivers-mux
eurorack-storage
eurorack-simulation
eurorack-platform-arduino
```

They contain no duplicate implementations. Their manifests depend on all individual libraries in
the category. They are useful for migration and broad native builds, but they are deliberately not
the recommended AVR default.

## Minimal Nano R3 configurations

### MCP4922 firmware without controls or storage

```ini
[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328
framework = arduino
build_unflags = -std=gnu++11
build_flags = -std=gnu++17 -Wall -Wextra -Wpedantic

lib_extra_dirs = lib/eurorack-framework/libraries
lib_ldf_mode = chain+
lib_deps =
    eurorack-driver-mcp4922
    eurorack-platform-arduino-gpio
    eurorack-platform-arduino-spi
```

The driver manifests pull `eurorack-compat`, `eurorack-io-digital`, `eurorack-io-result`, and
`eurorack-io-spi` transitively.

### Button and rotary encoder without LEDs or display

```ini
lib_deps =
    eurorack-control-momentary-button
    eurorack-control-rotary-encoder
```

### SSD1306 canvas without widgets

```ini
lib_deps =
    eurorack-display-monochrome-canvas
    eurorack-driver-ssd1306
    eurorack-platform-arduino-i2c
```

Add `eurorack-display-text`, `eurorack-display-glyphs`, or `eurorack-display-widgets` only when the
application uses those functions.

### EEPROM without native storage backends

```ini
lib_deps =
    eurorack-storage-interface
    eurorack-platform-arduino-eeprom
```

This does not select `FileStorage`, `MemoryStorage`, or `RecordStore`.

## Reference projects

`examples/projects/nano-r3-attenuverter/platformio.ini` demonstrates a small SPI/DAC build.

`examples/projects/nano-r3-oled-controller/platformio.ini` demonstrates selected controls, canvas,
widgets, one OLED controller, one GPIO expander, one analog multiplexer, and only the required
Arduino adapters.

Neither project includes framework `.cpp` files directly. The former `framework_subset.cpp` pattern
is obsolete and must not be reintroduced.

## Dependency validation

Run:

```bash
python tools/check-library-layout.py
```

The validator checks:

- one owner per public header,
- one unique library name per manifest,
- consistent package versions,
- known dependency names,
- declared dependencies for cross-library includes,
- absence of the obsolete root `include/` tree.

Native examples and test suites resolve their component closure from public includes and manifests.
This means an undeclared dependency can no longer be hidden by linking every framework source into
every executable.

## Measuring memory use

Granular libraries reduce the code made available to a build, but the actual result depends on the
compiler, optimization flags, templates, virtual interfaces, standard-library use, and linker garbage
collection. Measure the final firmware.

```bash
pio run -d <project>
pio run -d <project> -t size
```

For deeper analysis, inspect the ELF and map file with the AVR binutils installed by PlatformIO:

```bash
avr-size -C --mcu=atmega328p .pio/build/nanoatmega328/firmware.elf
avr-nm --size-sort --print-size .pio/build/nanoatmega328/firmware.elf
```

Track at least:

- flash used and remaining,
- static SRAM used,
- estimated worst-case stack margin,
- heap use or references to `operator new`, `malloc`, and `free`,
- interrupt and timer ownership.

## AVR compatibility layer

`eurorack-compat` supplies only the standard-library facilities required by framework code where the
classic Arduino AVR toolchain is incomplete. Headers select the shim only under `__AVR__`; other
platforms use the normal standard library.

The compatibility layer is shared infrastructure, not a reason to combine unrelated controls or
chips into one package.

## Arduino AVR SPI warning

Arduino AVR's bundled `SPI.h` uses an anonymous structure in a union. `-Wpedantic` reports this
non-standard construct. Firmware that enables `-Werror` and includes SPI may need:

```ini
build_flags =
    -Wpedantic
    -Wno-error=pedantic
```

This exception applies to vendored Arduino Core code, not framework-owned code.

## Migration from category libraries

Existing projects may continue to use category names temporarily. Migrate by replacing each category
with the concrete headers actually included by the firmware.

Example:

```ini
; Before
lib_deps =
    eurorack-controls
    eurorack-drivers-dac
    eurorack-platform-arduino

; After
lib_deps =
    eurorack-control-analog-input
    eurorack-control-cv
    eurorack-driver-mcp4922
    eurorack-platform-arduino-gpio
    eurorack-platform-arduino-spi
```

The public C++ include paths do not change.

## Current AVR limitations

Granular packaging does not claim that every component is already AVR-suitable. In particular,
drivers or storage models that still use dynamic containers remain unsuitable for strict heap-free
Nano R3 firmware until separately corrected. Timer, free-running ADC, interrupt edge capture, and
other target-specific real-time facilities must likewise be qualified independently.

The library split isolates these components so a project that does not use them no longer inherits
their implementation or dependencies.


## Quantizer support libraries

Declare only the components used by the module:

```ini
lib_deps =
    eurorack-driver-mcp4922
    eurorack-driver-tlc5947
    eurorack-control-analog-button-ladder
    eurorack-storage-fixed-slot
    eurorack-platform-avr-timer2
    eurorack-platform-avr-adc-scanner
    eurorack-platform-avr-external-edge
```

`eurorack-platform-avr-timer2` owns Timer2 and its compare-match ISR. `eurorack-platform-avr-adc-scanner` owns the ADC ISR and scans A5, A6, and A7 with AVCC or external AREF selection, prescaler 128, and one discarded conversion after each multiplexer change. `eurorack-platform-avr-external-edge` owns INT0 and INT1, corresponding to Nano pins D2 and D3. A project must not install competing ISR definitions for those vectors. SPI transfers remain foreground operations; do not call MCP4922 or TLC5947 methods from these ISRs.

The TLC5947 driver requires caller-owned arrays of `24 * deviceCount` 16-bit values and `36 * deviceCount` frame bytes. This makes SRAM use explicit and eliminates heap allocation. Call `initialize()` before enabling visible output.

## Real-time ownership and application rules

The AVR timer, ADC scanner, and external-edge libraries own hardware resources rather than sharing
them transparently with arbitrary Arduino APIs. A consuming firmware must document these choices.

- `eurorack-platform-avr-timer2` owns Timer2 and its compare interrupt. Do not combine it with
  libraries that reconfigure Timer2.
- `eurorack-platform-avr-adc-scanner` owns the ADC multiplexer, reference configuration, prescaler,
  conversion interrupt, and conversion lifecycle. Do not call `analogRead()` while it is active.
- `eurorack-platform-avr-external-edge` owns INT0 and INT1 when both channels are enabled.
- Device SPI transfers belong in the foreground scheduler, not in an ISR.
- EEPROM operations should be scheduled outside the 1 kHz critical section.

The recommended foreground order is: consume pending ticks, atomically copy ADC and edge snapshots,
update application state, write the DAC, and finally flush LED changes.

## TLC5947 startup selection

The TLC5947 driver supports `Silent`, `Sequential`, and `AllFlash` startup policies. Silent startup
remains the production default. Visible policies require `eurorack-io-time` and a concrete delay
provider such as `eurorack-platform-arduino-time`; they are blocking diagnostics and should not be
used after real-time processing has started. See `docs/guides/tlc5947.md`.

## Validation commands

```bash
python tools/check-library-layout.py
python tools/check-embedded-heap.py
python tools/run-native-tests.py
python tools/build-examples.py
```


## Resource-profile validation

The repository records exclusive AVR resources in `docs/architecture/framework-resources.json`. Validate a selected library profile before hardware integration:

```bash
python tools/check-resource-conflicts.py examples/profiles/avr-shared-spi.json
```

This check detects declared timer and vector collisions. It does not replace pin, voltage, address, or electrical validation.
