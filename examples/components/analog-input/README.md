# Analog Input

## Learning goal

Convert one raw ADC code into normalized unipolar and bipolar values.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- `AnalogInput` owns only calibration and the latest calculated snapshot.
- `update()` accepts an already acquired ADC code. Hardware sampling remains outside the model.
- `normalized` ranges from 0.0 to 1.0 and `bipolar` from -1.0 to +1.0.
- Samples outside the calibrated endpoints are clamped while diagnostic flags remain visible.

## Program flow

Create the model with raw endpoints, submit a midpoint sample, then inspect the immutable snapshot.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example analog-input
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

- Set `inverted` to true when the physical control direction is reversed.
- Replace the literal sample with an ADC backend in consuming firmware.
- Use measured calibration endpoints instead of assuming the full converter range.

## Common mistakes

- Do not confuse raw ADC codes with volts.
- Do not hide `belowRange` or `aboveRange`; they often reveal wiring or calibration faults.
- The class performs no smoothing. Use `Potentiometer` or another filter when required.

## Embedded integration

A real firmware project replaces literal samples with data obtained from
platform adapters or peripheral drivers. Keep the control model independent
from pin setup and electrical details.

Do not connect Eurorack signals directly to MCU pins. Protection, scaling,
biasing, buffering, current limiting, grounding, and calibration remain hardware
responsibilities.

## Related API

The public declaration is located below the owning `libraries/eurorack-control-*/include/eurorack/controls/` directory. Its
Doxygen comments define ownership, units, event lifetime, and error semantics.
