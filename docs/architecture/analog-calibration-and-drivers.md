# Analog Calibration and Additional Drivers

Step 12 adds an affine converter calibration and two hardware drivers.

`LinearCodeCalibration` converts between raw converter codes and voltage using
an explicit slope and offset. It can derive those values from two measured
points and reports range violations when converting voltage back to a code.

The DAC8568 driver provides eight buffered 16-bit channels and 32-bit SPI
command frames. Voltage scaling and output-stage behavior remain outside the
driver.

The SH1106 driver supports common 128x64 I2C OLED panels, converts the row-major
canvas into page data, and applies a configurable column offset.
