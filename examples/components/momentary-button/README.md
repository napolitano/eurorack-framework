# Momentary Button

## Learning goal

Debounce an active-low momentary button and observe a clean press edge.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Raw electrical level and logical pressed state are intentionally separate.
- The candidate level must remain stable for the configured debounce interval.
- `justPressed` and `justReleased` are valid only until the next `update()` call.
- Unsigned timestamp subtraction keeps debounce and hold timing valid across timer wraparound.

## Program flow

Reset in the released state, present a pressed candidate, then present it again after the debounce time.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example momentary-button
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

- Use `ActiveLevel::High` for active-high wiring.
- Set debounce to zero only for already conditioned digital signals.
- Copy edge events into an application queue when they must survive multiple loop iterations.

## Common mistakes

- Calling `update()` only when the pin changes prevents hold duration from advancing.
- Do not interpret the first sample as a synthetic edge.
- Mechanical switches still require suitable pull resistors.

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
