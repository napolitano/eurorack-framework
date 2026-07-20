# DIP Switch Bank

## Learning goal

Represent a fixed-size bank of maintained configuration switches and pack their states into a bit mask.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- The bank size is a compile-time constant from 1 through 32.
- Pole zero maps to bit zero in the returned mask.
- Each pole is independently debounced.
- No dynamic allocation is used.

## Program flow

Reset four poles from a boolean array and verify that poles zero and two produce mask `0x05`.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example dip-switch
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

- Use a different template size for the physical switch bank.
- Read poles from a GPIO expander before calling `update()`.
- Store the resulting mask in persistent configuration.

## Common mistakes

- Document the physical numbering direction of the DIP package.
- Do not assume printed switch numbers match zero-based software indices.
- A changing DIP bank may not be appropriate for live performance controls.

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
