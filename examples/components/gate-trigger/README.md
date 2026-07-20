# Gate and Trigger

## Learning goal

Apply gate hysteresis and create a timed trigger pulse.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Gate input uses separate rising and falling thresholds.
- Hysteresis prevents chatter near one threshold.
- Trigger output remains high for a configured duration.
- Time is supplied explicitly by the caller.

## Program flow

Raise the gate above its high threshold, start a trigger, then advance time beyond the pulse duration.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example gate-trigger
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

- Tune thresholds to the protected analog frontend.
- Use a hardware timer for tighter pulse timing.
- Convert the trigger snapshot into a digital-output request.

## Common mistakes

- The gate model accepts volts, not a raw digital pin.
- Sparse update calls make pulse completion late.
- Thresholds must match real hardware behavior.

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
