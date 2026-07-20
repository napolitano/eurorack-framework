# Illuminated Fader

## Learning goal

Combine a calibrated fader with RGB illumination that follows its position.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- The composite owns a `Fader` and `MulticolorLed`.
- `FollowPosition` maps normalized travel to master brightness.
- Minimum and maximum brightness are configurable.
- Color remains independent from automatic brightness.

## Program flow

Reset near midpoint and verify that the blue LED master brightness is approximately halfway.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example illuminated-fader
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

- Use inverse mode for decreasing illumination.
- Keep a nonzero minimum brightness for dark stages.
- Use Manual mode when the light represents state rather than position.

## Common mistakes

- Position-following light is not the same as a motorized fader.
- No gamma correction is applied.
- This model does not implement touch sensing.

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
