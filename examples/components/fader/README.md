# Fader

## Learning goal

Use fader-specific direction and calibration while reusing potentiometer processing.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- `Fader` wraps the potentiometer processor with panel-oriented terminology.
- `BottomToTop` means upward travel increases the logical value.
- `TopToBottom` reverses the logical direction without changing wiring.
- The returned snapshot has the same normalized and bipolar semantics as `Potentiometer`.

## Program flow

Construct a bottom-to-top fader, reset it at the midpoint, then inspect the resulting normalized position.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example fader
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

- Use measured raw endpoints to compensate for mechanical end stops.
- Reverse direction in configuration rather than swapping application arithmetic.
- Add smoothing only after testing the control's native noise.

## Common mistakes

- A linear fader does not imply a perceptually linear audio response.
- This model does not implement pickup or soft takeover.
- The example is a state-model demonstration, not an ADC wiring guide.

## Embedded integration

A real firmware project replaces literal samples with data obtained from
platform adapters or peripheral drivers. Keep the control model independent
from pin setup and electrical details.

Do not connect Eurorack signals directly to MCU pins. Protection, scaling,
biasing, buffering, current limiting, grounding, and calibration remain hardware
responsibilities.

## Related API

The public declaration is located below `include/eurorack/controls/`. Its
Doxygen comments define ownership, units, event lifetime, and error semantics.
