# Examples

The examples are executable documentation. They are intentionally small, but
must remain readable, buildable, and explicit about error handling.

## Directory structure

- `components/` contains native examples for one reusable library or driver.
  `tools/build-examples.py` compiles and executes every example in this tree.
- `projects/` contains complete PlatformIO applications for target boards.
- `profiles/` contains machine-readable resource profiles used by architecture
  checks.
- `arduino-platform/` and `momentary-button/` contain focused integration
  material retained for compatibility and introductory use.

## Component example conventions

Every component example has a `main.cpp` and a local `README.md`. Driver
examples use the test adapters in `components/driver_example_support.hpp` so
that protocol behavior can be demonstrated without Arduino headers or hardware.
These adapters are not production backends.

A useful example shall:

1. include the public library header;
2. use descriptive object and signal names;
3. construct and configure the component visibly;
4. perform at least one representative operation;
5. inspect or propagate every relevant `IoResult`;
6. return a non-zero process status when the demonstrated contract fails.

Build all native component examples with:

```bash
python tools/build-examples.py
```

Target projects are built from their own directory with PlatformIO, for example:

```bash
cd examples/projects/nano-r3-oled-controller
pio run
```

## Hardware adaptation

Replace the native adapters with the appropriate platform libraries when using
an example on hardware. The corresponding library and hardware documentation
identify bus settings, ownership rules, electrical constraints, and required
pins. Never infer electrical limits from a native example.
