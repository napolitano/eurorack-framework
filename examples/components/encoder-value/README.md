# Encoder Value

## Learning goal

Apply encoder detents to a bounded application value.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- `EncoderValue` is independent from quadrature decoding.
- It applies signed detents using a configured step.
- Boundary behavior can clamp or wrap.
- The snapshot records whether normalization changed the candidate.

## Program flow

Start at five, apply three detents with step one, and verify the resulting value is eight.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example encoder-value
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

- Use a larger step for coarse adjustment.
- Use wrap mode for cyclic selections.
- Combine it with `RotaryEncoder::snapshot().delta`.

## Common mistakes

- Do not feed raw A/B states into this class.
- Choose signed limits that safely contain all intermediate arithmetic.
- Wrapping is suitable for menus but often wrong for physical gain controls.

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
