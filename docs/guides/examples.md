# Working With Examples

Examples are part of the supported learning surface of the project. They must
remain compilable, readable, narrowly scoped, and explicit about hardware
boundaries.

## Two example levels

### Component examples

`examples/components/` contains small native programs. Each isolates one
framework abstraction and uses literal samples.

Every component example contains:

- a documented `main.cpp`
- a detailed README
- a deterministic success condition
- a command that builds and executes it
- variations and common mistakes
- embedded integration notes

Build all component examples:

```bash
python tools/build-examples.py
```

### Project examples

`examples/projects/` contains complete but deliberately simple applications.

A project example must document:

- intended learning outcome
- hardware contract
- signal flow
- pin assignment
- platform configuration
- code execution path
- calibration
- limitations
- fault behavior
- troubleshooting
- exercises for extension

The first project example is the Arduino Nano R3 one-channel attenuverter.

## Documentation quality

An example is not adequately documented merely because the code compiles.
Comments and README text must explain why each abstraction is selected, which
state changes are expected, and which responsibilities remain outside the
framework.

Examples must never imply that MCU pins are electrically compatible with
Eurorack jacks.
