# Hypothetical Hardware Contract

This file does not define a buildable module. It documents the electrical
assumptions made by the software example.

## Jacks

The conceptual front panel contains:

```text
one bipolar CV input jack
one bipolar CV output jack
```

Neither jack is connected directly to the Arduino Nano.

## Hypothetical input path

The software assumes an external stage with the nominal transfer function:

```text
Vadc = 0.5 * Vin + 2.5 V
```

That would map:

```text
-5 V input -> 0 V ADC
 0 V input -> 2.5 V ADC
+5 V input -> 5 V ADC
```

A real design would need to consider:

- input impedance
- series resistance
- overvoltage and negative-voltage protection
- op-amp input common-mode range
- output swing into the ADC
- anti-alias or noise filtering
- ADC source impedance
- reference accuracy
- fault cases beyond the nominal -5 V to +5 V range

The example does not choose resistor values or an op-amp.

## Hypothetical potentiometers

Two linear potentiometers are assumed:

```text
A1 attenuation
A2 balance
```

Each is notionally connected between a clean 5 V rail and ground, with its
wiper feeding the ADC.

A real design might include:

- center-detent potentiometers
- small wiper capacitors
- series resistors
- measured endpoint calibration
- measured center calibration
- software filtering

## Hypothetical DAC

The example assumes:

```text
MCP4922 channel A
VREF = 4.096 V
gain = 1x
```

That produces an unsigned voltage from approximately 0 V to 4.096 V.

A real circuit would need:

- a reference source
- supply decoupling
- SPI wiring
- defined LDAC behavior
- startup-state analysis
- grounding and return-current planning

## Hypothetical output path

The software assumes:

```text
Vout = 2.44140625 * (Vdac - 2.048 V)
```

That would map:

```text
0.000 V DAC -> -5 V output
2.048 V DAC ->  0 V output
4.096 V DAC -> +5 V output
```

A real implementation would likely use an op-amp or equivalent analog stage.

It would also need:

- output-current limiting
- cable-capacitance stability analysis
- output impedance
- short-circuit behavior
- protection against external voltage
- startup and shutdown behavior
- calibration
- suitable operation from the chosen supply rails

No specific op-amp is prescribed because this example is about software
structure, not a complete analog design.

## Hypothetical bipolar LED

The code assumes:

```text
three-pin red/green LED
common cathode
one resistor per color die
red on D5
green on D6
```

A common-anode LED would require inverted PWM behavior.

A two-pin bipolar LED would require a different driver topology and cannot be
treated as two independent PWM channels.

## Power

A real module would require a proper Eurorack power entry and regulated digital
supply. The Nano should not be powered from raw +12 V through an improvised
connection.

The example deliberately omits:

- reverse-polarity protection
- rail filtering
- regulator selection
- thermal calculations
- connector design
- PCB grounding
- decoupling layout

Those omissions are acceptable only because no complete hardware is being
claimed.
