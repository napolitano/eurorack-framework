# Arduino Platform Adapters

Step 11 provides the first real platform adapter set.

Included adapters:

- digital input
- digital output
- bidirectional digital pin
- ADC input
- PWM or core-provided DAC output through `analogWrite`
- monotonic time and microsecond delay
- SPI
- I2C

All adapters are guarded by `ARDUINO`. Native and simulator builds therefore do
not require Arduino headers.

`ArduinoPwmOutput` deliberately describes raw `analogWrite` codes. On classic
AVR boards this is PWM, not a precision DAC. A filtered PWM output is not a
substitute for a calibrated Eurorack CV DAC.

The adapters accept Arduino core objects such as `SPIClass` and `TwoWire` by
reference, allowing alternate hardware peripherals on cores that expose them.
