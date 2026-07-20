# Potentiometer

## Learning goal

Process a noisy panel potentiometer with calibration, deadband, inversion, and exponential smoothing.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- `reset()` establishes the initial state without reporting a movement event.
- `update()` applies clamping, optional inversion, smoothing, and deadband.
- `changed` reports whether the public normalized value moved far enough to matter.
- The raw sample and range diagnostics are updated even when deadband suppresses a value change.

## Program flow

Reset at the minimum code, provide a new sample, and inspect the smoothed normalized and bipolar values.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example potentiometer
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

- Set `smoothingFactor` to 1.0 for immediate response.
- Use a lower smoothing factor for noisy controls.
- Increase deadband only enough to suppress visible jitter.

## Common mistakes

- Excessive smoothing creates noticeable lag.
- An oversized deadband produces a control that appears to stick.
- Calibration endpoints should be measured on the actual hardware.

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
