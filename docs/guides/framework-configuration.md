# Editable Eurorack Configuration

The file `libraries/eurorack-core/include/eurorack_config.hpp` is the human-editable configuration
surface of the framework. It contains ordinary named constants, grouped by
purpose and documented in volts, milliseconds, hertz, and microseconds.

A user does not need to edit `framework_config.hpp`.

## Default operating assumptions

| Area | Default |
|---|---:|
| Nominal audio | -5 V to +5 V |
| Accepted general CV input | -10 V to +10 V |
| Generated general CV output | -5 V to +5 V |
| Accepted pitch CV | -5 V to +10 V |
| Generated pitch CV | 0 V to +10 V |
| Pitch scaling | 1 V/octave |
| Chromatic divisions | 12 steps/octave |
| Gate definitely low | <= 1 V |
| Gate definitely high | >= 2 V |
| Gate/trigger/clock output | 0 V / +5 V |
| Default trigger pulse | 5 ms |
| Button debounce | 20 ms |
| Long press | 800 ms |
| Double click | 300 ms |
| Control update | 1000 Hz |
| UI refresh | 60 Hz |
| Simulator step | 1000 us |

These are interoperability-oriented software defaults, not universal Eurorack
laws. Doepfer documents audio around 10 V peak-to-peak and gate, trigger, and
clock signals typically at 0 V / +5 V. Module-specific CV ranges vary
substantially, which is why input acceptance and generated output ranges are
separate.

## Editing the library default

Edit only values in:

```text
libraries/eurorack-core/include/eurorack_config.hpp
```

For example:

```cpp
inline constexpr float CV_OUTPUT_MIN_VOLTS = -8.0F;
inline constexpr float CV_OUTPUT_MAX_VOLTS = 8.0F;
inline constexpr std::uint32_t BUTTON_DEBOUNCE_MILLISECONDS = 15U;
```

## Overriding from a consuming project

A consuming project should normally provide its own file, for example:

```text
include/my_module_config.hpp
```

The replacement file declares the same constants in the same
`eurorack_config` namespace. The project's `platformio.ini` selects it:

```ini
build_flags =
    -DEURORACK_FRAMEWORK_CONFIG_FILE=\<my_module_config.hpp\>
```

Alternatively, a quoted relative header can be used when supported by the build
environment:

```ini
build_flags =
    -DEURORACK_FRAMEWORK_CONFIG_FILE=\"my_module_config.hpp\"
```

The framework internally includes the selected file and constructs the typed
`eurorack::core::eurorackDefaults` object from it. Controls, drivers, tests,
display code, and a simulator therefore consume one consistent set of values.

## Validation

The selected values are checked with:

```cpp
static_assert(eurorack::core::isValid(
    eurorack::core::eurorackDefaults));
```

Validation checks logical consistency. It cannot verify that the analog hardware
can safely accept or generate the configured voltages.
