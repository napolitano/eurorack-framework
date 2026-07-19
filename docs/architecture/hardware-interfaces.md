# Generic Hardware Interfaces

The `eurorack::io` layer is the lowest public hardware abstraction in the
framework. It defines contracts but contains no Arduino, STM32, RP2040, ESP32,
Renesas, operating-system, or simulator implementation.

## Interfaces

- `DigitalInput`, `DigitalOutput`, and `BidirectionalDigitalPin`
- `AnalogInputChannel` and `AnalogOutputChannel`
- `TimeSource` and `DelayProvider`
- `SpiBus`
- `I2cBus`
- common `IoResult` status values

## Boundary

Hardware backends translate platform facilities into these contracts:

```text
Arduino / STM32 / RP2040 / desktop simulator
                         |
                         v
                 eurorack::io interfaces
                         |
                         v
        IC drivers, controls, storage, display drivers
```

Chip-select is intentionally not hidden inside `SpiBus`. A device driver owns a
separate `DigitalOutput`, which allows unusual chip-select routing through GPIO
expanders or simulator models.

I2C addresses are always seven-bit values. Callers must not pre-shift the address
or include the read/write bit.

Analog interfaces exchange raw converter codes. Calibration and conversion into
volts belong to higher layers because those calculations depend on divider,
reference, op-amp, and calibration data rather than the MCU API itself.

`DelayProvider` is separate from `TimeSource` because most real-time framework
logic should remain nonblocking. Blocking delays are reserved for hardware reset
and initialization sequences where a device datasheet explicitly requires them.
