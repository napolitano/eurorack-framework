# Eurorack Framework Manual

> **Development status**
>
> This framework is under active development and is not intended for production
> firmware. APIs, persistent formats, timing behavior, and hardware assumptions
> may still change. Anyone using it before a stable release does so at their own
> risk and remains responsible for electrical design, validation, calibration,
> maintenance, and regulatory compliance.

This manual is the primary project documentation. It is written in Markdown so
it remains readable directly in the repository, in an editor, and on common
source-hosting platforms.

The generated PDF combines this manual with the API reference extracted from
the documented C++ declarations.

## Start here

1. [Getting started](getting-started.md)
2. [How the framework is organized](project-tour.md)
3. [Core concepts](core-concepts.md)
4. [Development workflow](development-workflow.md)
5. [API reference guide](api-reference-guide.md)
6. [Known limitations](known-limitations.md)

## What this framework provides

The framework supplies reusable infrastructure for Eurorack firmware:

- input and output abstractions
- buttons, switches, encoders, LEDs, potentiometers, and faders
- analog, CV, gate, trigger, and jack models
- display canvas, drawing, text, and widgets
- peripheral drivers
- persistence
- Arduino adapters
- deterministic native simulation

Concrete musical behavior, product-specific signal processing, module menus,
and application state belong in consuming firmware projects rather than this
repository.

## Reading strategy

New contributors should read the manual in order before browsing the API
reference. The API reference explains individual types; the manual explains why
those types exist, how they fit together, and which responsibilities remain with
the application and hardware.


## Component and project examples

Minimal compilable examples are in `examples/components`. The first complete
application example is `examples/projects/nano-r3-attenuverter`.

## Hardware reference

- [Supported ICs and hardware families](../hardware/index.md)
