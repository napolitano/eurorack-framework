# Bicolor LED

## Learning goal

Translate a logical two-color request into topology-specific terminal drive states.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Common-anode, common-cathode, and bipolar packages behave differently.
- The example uses a common-cathode package.
- A bipolar two-lead LED requires time multiplexing for a mixed color.
- The snapshot exposes both logical color and required pin-drive intent.

## Program flow

Create the LED as Off, request Color A, and verify the effective color.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example bicolor-led
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

- Inspect `pinA` and `pinB` in a hardware adapter.
- Use `Mixed` only with a suitable multiplexing strategy.
- Map Color A and Color B to the actual package colors in project documentation.

## Common mistakes

- Do not assume package pin order from LED color names.
- Common-anode wiring reverses current logic.
- Each die still needs suitable current limiting.

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
