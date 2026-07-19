# Design Decisions

## Why balance is asymmetric scaling

The requested balance behavior is interpreted as different positive and
negative output ranges while keeping zero fixed.

A DC offset was rejected because it would shift the entire signal and change the
electrical zero point.

## Why balance follows the attenuated polarity

Attenuation may invert the signal. Applying balance afterwards means balance
describes the actual output polarity and matches the LED indication.

## Why center dead zones are explicit

Mechanical center detents and ADC midpoints are imperfect. Without a dead zone,
center positions would often leak a small gain or imbalance.

Rescaling outside the dead zone preserves full endpoint travel.

## Why use a pure model header

The core arithmetic is independent from Arduino and the framework's hardware
adapters. This improves readability and allows direct native testing.

## Why use MCP4922

Nano R3 has no true DAC. MCP4922 provides a straightforward 12-bit SPI output
with an external reference.

PWM is used only for the LED, where ripple and precision are not critical.

## Why assume an op-amp but not specify one

A real bipolar input and output require analog conditioning. Selecting an op-amp
depends on the actual supply rails, input common-mode range, output swing, load,
offset, bias current, speed, stability, availability, and cost.

The project demonstrates the software contract, not a finished analog design.

## Why use floating point

Voltage-domain arithmetic is easy to read and verify.

ATmega328P implements floating point in software, so a production design might
replace it with tested fixed-point arithmetic after performance measurement.

## Why use two independent PWM pins for the LED

A three-pin red/green LED can display polarity cleanly with one PWM channel per
die. A two-pin bipolar LED would need an H-bridge-like driver or alternating pin
directions and is less clear for a teaching example.
