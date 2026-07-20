# Illuminated Button

## Learning goal

Combine a debounced momentary button with automatic RGB illumination.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- The composite owns both button and LED state models.
- `LitWhilePressed` maps stable pressed state to the configured active color.
- Input and LED hardware remain separate.
- Manual mode leaves color changes entirely to application logic.

## Program flow

Reset released, press the active-high button, and verify that green illumination becomes effective.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example illuminated-button
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

- Use `LitWhileReleased` for inverse feedback.
- Set master brightness independently from color.
- Use Manual mode for application-specific status colors.

## Common mistakes

- The LED does not electrically debounce the switch.
- Automatic color follows stable state, not the raw pin.
- A real RGB button usually needs several driver channels.

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
