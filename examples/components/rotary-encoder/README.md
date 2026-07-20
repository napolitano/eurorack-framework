# Rotary Encoder

## Learning goal

Decode one complete quadrature detent from sampled A/B states.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Quadrature uses a legal Gray-code transition sequence.
- The example submits four transitions for one nominal detent.
- `delta` reports movement from only the most recent update.
- Impossible two-bit jumps increment `invalidTransitionCount`.

## Program flow

Reset at 00, walk through a full Gray-code cycle, then inspect position, delta, and direction.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example rotary-encoder
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

- Set `inverted` when logical direction is opposite to panel expectation.
- Adjust `transitionsPerDetent` for the actual encoder.
- Sample from interrupts when polling cannot reliably capture every transition.

## Common mistakes

- The physical encoder's transition order may be opposite to the example.
- Slow polling can miss states and produce invalid transitions.
- This class does not include a pushbutton; that is a separate composite control.

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
