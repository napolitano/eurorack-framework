# SSD1306 Driver

The SSD1306 driver is the first concrete display-controller implementation.

It consumes the framework's `I2cBus` interface and accepts a
`MonochromeCanvas`. The canvas uses row-major MSB-first pixels, while SSD1306
display RAM uses vertical eight-pixel pages. `flush()` performs this conversion.

Supported operations:

- controller initialization
- 128x32 and 128x64 panel heights
- horizontal addressing mode
- complete framebuffer transfer
- contrast
- hardware inversion
- display on and off
- segment and COM scan remapping
- internal charge pump selection

The driver transfers display data in bounded I2C chunks and contains no Arduino
`Wire` dependency.
