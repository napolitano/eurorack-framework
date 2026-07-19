# On-Off-(On) Switch

## Learning goal

Model a maintained On position, center Off, and spring-return momentary On position.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- Two independent contacts are required.
- Neither asserted means center Off.
- Both asserted is reported as `Invalid` rather than guessed.
- The momentary side is a position, not an automatically timed pulse.

## Program flow

Reset with neither contact active, assert the momentary contact, then inspect the derived position.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example on-off-momentary-switch
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

- Configure separate active levels for the two contacts.
- Translate `MomentaryOn` into a trigger in application logic.
- Log `Invalid` states to diagnose overlap or wiring faults.

## Common mistakes

- Do not wire both contacts to one digital input.
- Contact overlap may occur mechanically and should be tested.
- The model does not know whether a contact is physically spring-loaded.

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
