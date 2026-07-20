# Digital LED

## Learning goal

Translate a logical LED state into an active-high electrical output level.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Logical On/Off is separate from GPIO polarity.
- `rawOutputHigh()` already includes active-high or active-low configuration.
- Transition flags and counts track accepted logical changes.
- The model performs no GPIO access.

## Program flow

Construct an initially off active-high LED, turn it on, and inspect logical and raw output state.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example digital-led
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

- Use active-low for current-sinking or inverted driver stages.
- Write `rawOutputHigh()` through a platform digital-output adapter.
- Use a dedicated LED driver for PWM brightness.

## Common mistakes

- Always include electrical current limiting.
- Logical brightness is binary in this class.
- The software request does not prove that the physical LED is illuminated.

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
