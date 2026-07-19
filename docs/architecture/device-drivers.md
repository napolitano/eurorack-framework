# Device Driver Architecture

Step 5 adds generic LED interfaces and reusable drivers for MCP4922, MCP23017, TLC5916, TLC5947, 74HC595, and 74HC165. Drivers depend only on framework SPI, I2C, and digital-pin interfaces. Buffered devices separate state mutation from `flush()`. TLC5916 maps any nonzero logical brightness to on, while TLC5947 maps 16-bit brightness to 12-bit PWM.
