# Framework Boundary

## Purpose

Eurorack Framework supplies reusable infrastructure. It does not implement a
specific Eurorack module.

## Included

- controls and state machines;
- voltage, gate, trigger, clock, and jack abstractions;
- bus, pin, ADC, DAC, time, and storage interfaces;
- drivers for external integrated circuits;
- display canvases, drawing primitives, widgets, and controller drivers;
- persistence infrastructure;
- deterministic simulation infrastructure.

## Excluded

- product-specific musical algorithms and domain models;
- sequencer, tuner, oscillator, envelope, or LFO engines;
- module-specific menus, presets, calibration flows, and UI state;
- complete module firmware.

Those features belong to consuming repositories. They may use the framework
without becoming part of it.

## Review rule

A component belongs in this framework only when at least two materially
different module applications could use it without adopting application-specific
terminology or behavior.
