# Control Voltage Models

## Learning goal

Keep application logic in volts while retaining clamping diagnostics.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- `CvInput` accepts a calibrated voltage, not an ADC code.
- `CvOutput` stores requested and range-limited effective voltage.
- The example uses a bipolar -5 V to +5 V range.
- Hardware conversion and protection remain outside both models.

## Program flow

Update the input with +2.5 V, request -2.5 V at the output, then inspect normalized and effective states.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example cv
```

The tool compiles this example together with the framework implementation using
C++17 and strict warnings, then executes the resulting native binary.

Build every component example:

```bash
python tools/build-examples.py
```

## Code walkthrough

Read `main.cpp` from top to bottom. The comments explain:

1. which configuration values are chosen
2. which raw or logical samples are supplied
3. which method advances the state model
4. which snapshot field proves the expected behavior

## Variations to try

- Use narrower ranges for unipolar modulation.
- Combine with `AnalogCalibration` for converter-code translation.
- Log clamp flags during hardware bring-up.

## Common mistakes

- The class does not produce a physical voltage.
- Clamping protects software assumptions, not hardware.
- Nominal Eurorack ranges are not universal.

## Embedded integration

A real firmware project replaces literal samples with data obtained from
platform adapters or peripheral drivers. Keep the control model independent
from pin setup and electrical details.

Do not connect Eurorack signals directly to MCU pins. Protection, scaling,
biasing, buffering, current limiting, grounding, and calibration remain hardware
responsibilities.

## Related API

The public declaration is located below `include/eurorack/controls/`. Its
Doxygen comments define ownership, units, event lifetime, and error semantics.
