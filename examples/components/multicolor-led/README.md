# Multicolor LED

## Learning goal

Store a logical RGB color and apply a 16-bit master-brightness scale.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Each color channel uses a logical 16-bit intensity.
- Master brightness scales all channels with rounded integer arithmetic.
- The class is topology-neutral.
- Gamma correction and optical calibration remain outside the model.

## Program flow

Create an orange-biased color, halve master brightness, and compare effective red and green channels.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example multicolor-led
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

- Forward the three effective values to PWM or constant-current channels.
- Apply gamma correction in a dedicated rendering layer.
- Calibrate channels to compensate for unequal LED efficiency.

## Common mistakes

- Equal numeric RGB values rarely produce equal perceived brightness.
- The model does not know common-anode or common-cathode polarity.
- Maximum logical brightness does not define safe LED current.

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
