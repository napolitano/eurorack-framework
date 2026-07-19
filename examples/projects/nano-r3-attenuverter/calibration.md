# Hypothetical Calibration

No hardware is provided, so these steps describe how a future implementation
would replace nominal constants with measurements.

## Input path

Assumed relationship:

```text
Vadc = INPUT_SCALE * Vin + INPUT_OFFSET_VOLTS
```

Using two known input voltages and measured ADC-pin voltages:

```text
INPUT_SCALE =
    (Vadc2 - Vadc1) / (Vin2 - Vin1)

INPUT_OFFSET_VOLTS =
    Vadc1 - INPUT_SCALE * Vin1
```

## ADC reference

Measure the actual ADC reference under normal operating conditions and replace:

```cpp
ADC_REFERENCE_VOLTS
```

USB 5 V should not be assumed precise.

## Potentiometer endpoints and centers

For both A1 and A2, record:

```text
minimum raw code
center-detent raw code
maximum raw code
```

The current example assumes 0, approximately 512, and 1023.

A more refined implementation could use piecewise calibration around the
measured center instead of only a symmetric dead zone.

## DAC and output path

Assumed relationship:

```text
Vout = OUTPUT_SCALE * (Vdac - DAC_MIDPOINT_VOLTS)
```

Using measured DAC and output voltages:

```text
OUTPUT_SCALE =
    (Vout2 - Vout1) / (Vdac2 - Vdac1)

DAC_MIDPOINT_VOLTS =
    Vdac1 - Vout1 / OUTPUT_SCALE
```

## LED

LED calibration is perceptual rather than electrical.

Possible refinements include:

- different gamma per color
- separate maximum brightness per die
- compensation for unequal red and green efficiency
- minimum visible PWM threshold
- clipping indication

The current example uses one shared gamma exponent and assumes equal safe PWM
ranges.
