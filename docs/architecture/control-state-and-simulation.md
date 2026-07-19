# Control State, Rendering, and Simulation

## Decision

Framework controls do not render themselves and do not depend on a display,
graphics library, Arduino API, or desktop UI toolkit.

Each control instead follows a deterministic update-and-snapshot model:

```text
Raw hardware or simulated input
              |
              v
       Control state machine
              |
              v
      Immutable state snapshot
         /              \
        v                v
Application logic   Display or simulator
```

For `MomentaryButton`, the application supplies:

- the sampled electrical level;
- a monotonic timestamp in milliseconds.

The component supplies:

- the stable pressed state;
- accepted press and release edges;
- current hold duration;
- most recently completed press duration;
- accepted transition count.

## Why this supports a simulator

A simulator can feed mouse, keyboard, MIDI, scripted, or network input into the
same `update()` method used by physical hardware. It can then render the returned
snapshot without replacing or duplicating the button behavior.

This provides useful separation:

- hardware adapters translate GPIO into raw levels;
- controls implement physical and interaction semantics;
- application firmware assigns domain meaning;
- renderers visualize snapshots;
- simulators generate raw inputs and advance a virtual clock.

## Display abstraction

A display API should not be added to `MomentaryButton`. Instead, a future display
layer should consume snapshots through application-specific view models.

A possible later shape is:

```cpp
struct ButtonViewModel {
    const char* label;
    bool pressed;
    bool justPressed;
    bool justReleased;
    std::uint32_t heldForMs;
};
```

The framework may later provide generic inspection metadata or snapshot visitors,
but physical controls should remain usable without any renderer.

## Event delivery

The current edge flags describe the latest `update()` call. This is deliberate:

- no heap is required;
- no callback lifetime rules are introduced;
- interrupt and main-loop ownership remain explicit;
- tests can inspect deterministic state.

Applications that can miss updates should copy snapshots into a bounded,
application-owned event queue. A generic fixed-capacity event queue belongs in
`core`, not inside each control.

## Output controls

Output controls follow the same separation. `DigitalLed` stores logical state
and exposes the electrical output level required by active-high or active-low
wiring.

A hardware adapter writes `rawOutputHigh` to a GPIO. A simulator ignores that
electrical detail and renders `on`. Both consume the same state object.

```text
Application state
       |
       v
   DigitalLed
    /      \
   v        v
GPIO level  Visual renderer
```

The LED does not read a button directly. Application code connects controls:

```cpp
button.update(rawLevelHigh, nowMs);
led.setOn(button.snapshot().pressed);
```

This avoids hidden coupling and allows the simulator to inspect or replace every
connection explicitly.
