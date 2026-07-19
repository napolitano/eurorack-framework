# Simulation Architecture

Step 10 adds deterministic platform-independent simulation components.

Included components:

- virtual monotonic time and delay
- virtual digital input, output, and bidirectional pins
- virtual ADC and DAC channels
- inspectable SPI and I2C buses
- queued bus responses and error injection
- deterministic input scenarios
- PBM and ASCII canvas export

The simulation layer implements the same interfaces as real platform adapters.
Framework controls and drivers therefore run unchanged in native tests or
desktop tooling.

The simulation layer intentionally has no GUI dependency. A later desktop
frontend may consume snapshots, bus records, and exported canvas data.
