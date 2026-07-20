# Toggle Switch

## Learning goal

Debounce a maintained two-position switch and expose On/Off transition events.

## Why this example exists

This program is deliberately small. It isolates one framework abstraction so
that its state transitions and snapshot semantics are visible without Arduino,
GPIO setup, peripheral drivers, or a complete module application.

It is an executable example rather than pseudocode. A successful process exit
confirms the expected result.

## Concepts demonstrated

- The switch uses maintained-state terminology instead of button terminology.
- Debounce behavior is delegated to the existing button state machine.
- `justTurnedOn` and `justTurnedOff` are one-update events.
- `isOn()` returns the stable logical state without sampling hardware.

## Program flow

Reset in Off, present an On level, wait for the debounce interval, and read the accepted state.

The final `return` statement acts as a minimal executable assertion. Exit code
zero means the demonstrated condition was met.

## Build and run

From the repository root:

```bash
python tools/build-examples.py --example toggle-switch
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

- Use active-low configuration for a switch connected to ground with a pull-up.
- Set debounce to zero for clean logic from another IC.
- Use `OnOffMomentarySwitch` when the hardware has three positions.

## Common mistakes

- The model does not configure pull resistors.
- A maintained switch can still bounce.
- Repeatedly setting the same level does not create another transition event.

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
