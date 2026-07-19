# Arduino Nano R3 One-Channel Attenuverter With Balance

This example shows how a small digital Eurorack utility could be structured
around an Arduino Nano R3. It is a software and architecture example. No
complete hardware design, schematic, PCB, or bill of materials is claimed.

The hypothetical module has:

- one bipolar CV input jack
- one bipolar CV output jack
- one center-detent attenuation potentiometer
- one center-detent balance potentiometer
- one red/green bipolar status LED
- one MCP4922 DAC channel
- hypothetical protected input and op-amp output stages

## Control concept

### Attenuation

The first potentiometer produces a signed gain:

```text
full left    -1.0   fully inverted
center        0.0   output muted
full right   +1.0   original polarity
```

A small software dead zone around center produces an exact zero even when the
mechanical detent and ADC midpoint are imperfect.

Values outside the dead zone are rescaled so the remaining travel still reaches
exactly -1.0 and +1.0.

### Balance

The second potentiometer does not add a DC offset. It changes how strongly the
positive and negative sides of the already attenuated signal are passed.

```text
full left     positive side suppressed, negative side full
center        positive and negative sides equal
full right    positive side full, negative side suppressed
```

The exact mapping is:

```text
balance  0.00 -> positive scale 1.00, negative scale 1.00
balance +0.75 -> positive scale 1.00, negative scale 0.25
balance -0.75 -> positive scale 0.25, negative scale 1.00
balance +1.00 -> positive scale 1.00, negative scale 0.00
balance -1.00 -> positive scale 0.00, negative scale 1.00
```

This creates an asymmetrically attenuated bipolar range without moving the
electrical zero point.

## Signal-processing order

```text
hypothetical bipolar input jack
        |
        v
hypothetical protection, scaling, and 2.5 V bias
        |
        v
Nano ADC A0
        |
        v
calibrated input voltage
        |
        +---- A1 attenuation: -1 to +1
        |
        v
signed attenuation
        |
        +---- A2 balance: positive-side versus negative-side scale
        |
        v
range limiting
        |
        v
MCP4922 channel A
        |
        v
hypothetical bipolar op-amp output stage
        |
        v
hypothetical output jack
```

The central calculation is:

```cpp
attenuated = inputVolts * attenuation;

output = attenuated >= 0.0F
    ? attenuated * positiveScale
    : attenuated * negativeScale;
```

## Worked examples

With attenuation at +0.5 and balance centered:

```text
input +4 V -> output +2 V
input -4 V -> output -2 V
```

With attenuation at +0.5 and balance at +0.75:

```text
positive scale = 1.00
negative scale = 0.25

input +4 V -> output +2.0 V
input -4 V -> output -0.5 V
```

With attenuation at -1.0 and balance at -0.50:

```text
positive scale = 0.50
negative scale = 1.00

input +3 V -> attenuated -3 V -> output -3.0 V
input -3 V -> attenuated +3 V -> output +1.5 V
```

Balance is applied after inversion. It therefore follows the polarity of the
actual attenuated output, which is also what the LED displays.

## Bipolar LED

The example assumes a hypothetical three-pin common-cathode red/green LED:

```text
positive output -> green
negative output -> red
near zero       -> off
```

Brightness follows the absolute requested output voltage. A modest gamma curve
prevents low voltages from appearing disproportionately bright.

The LED represents the output requested by the software after range limiting.
It does not measure the physical voltage at the output jack.

## Why a DAC is used

Arduino Nano R3 has no true analog DAC. `analogWrite()` produces PWM, which
would require filtering and would trade ripple against response time.

The example therefore uses MCP4922 channel A:

```text
12-bit resolution
SPI transport
external voltage reference
unsigned 0 V to VREF output
```

The DAC remains hypothetical in the sense that this repository provides no
complete hardware implementation.

## Why an op-amp is assumed

The DAC produces only a positive voltage. A bipolar output such as -5 V to +5 V
requires an external stage that subtracts a midpoint and applies gain.

The example assumes:

```text
Vout = 2.44140625 * (Vdac - 2.048 V)
```

An op-amp or equivalent analog stage is therefore required for a real device.

A suitable op-amp would also be expected in the hypothetical input stage for
protection, buffering, scaling, and level shifting. This project intentionally
does not specify a final circuit or component selection.

## Pin assignment

```text
A0   conditioned bipolar CV input
A1   attenuation potentiometer wiper
A2   balance potentiometer wiper

D5   red LED PWM
D6   green LED PWM

D10  MCP4922 chip select
D11  SPI MOSI
D13  SPI clock
```

Both potentiometers are assumed to be linear and connected between clean 5 V
and ground. Center-detent types are useful but still require software dead zones
or calibration.

## Build

From this directory:

```bash
pio run
```

Upload:

```bash
pio run --target upload
```

No serial interface is required.

## Important limitations

This remains an Unreleased Alpha teaching example.

It is not intended for:

- direct connection between Eurorack jacks and Nano pins
- audio processing
- precision pitch CV
- commercial hardware
- safety-critical use
- production-ready analog design
- claims about a specific op-amp, reference, regulator, or protection circuit

The code describes how the software could be organized. The README describes
the hypothetical electrical contract needed to make that software meaningful.

## Documentation map

Read in this order:

1. `README.md` - behavior and architecture
2. `control-behavior.md` - exact attenuation and balance mapping
3. `code-walkthrough.md` - firmware execution
4. `hardware.md` - hypothetical hardware contract
5. `calibration.md` - how nominal transfer constants would be measured
6. `design-decisions.md` - why the example uses this structure
7. `troubleshooting.md` - conceptual fault isolation
