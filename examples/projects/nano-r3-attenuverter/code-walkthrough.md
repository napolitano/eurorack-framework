# Firmware Walkthrough

## 1. Pure processing model

`src/attenuverter_model.hpp` contains the application arithmetic without Arduino
or framework dependencies.

It provides:

```cpp
applyCenteredDeadZone()
calculateSideScales()
process()
```

Keeping this arithmetic separate makes it possible to reason about the behavior
without ADC, SPI, PWM, or analog hardware.

## 2. Input acquisition

The loop reads three ADC channels:

```text
A0 conditioned CV input
A1 attenuation potentiometer
A2 balance potentiometer
```

Each channel is sampled eight times and averaged.

This is a simple noise-reduction measure. It does not create guaranteed extra
resolution and does not replace proper analog filtering.

## 3. Voltage-domain input

The A0 code is converted into an assumed ADC voltage:

```text
Vadc = code * ADC_REFERENCE_VOLTS / 1023
```

The hypothetical external input transfer is then reversed:

```text
Vin = (Vadc - INPUT_OFFSET_VOLTS) / INPUT_SCALE
```

`CvInput` receives the resulting voltage and limits it to the declared operating
range while retaining range diagnostics.

## 4. Attenuation control

A1 is converted by `AnalogInput` into a bipolar value.

The application then applies the center dead zone:

```cpp
attenuation = applyCenteredDeadZone(
    attenuationControl.snapshot().bipolar,
    ATTENUATION_DEAD_ZONE);
```

The result is a signed gain from -1 to +1.

## 5. Balance control

A2 follows the same conversion and dead-zone process.

The balance value controls side scales:

```text
negative balance -> positive side reduced
zero balance     -> both sides full
positive balance -> negative side reduced
```

## 6. Processing

The pure model first performs signed attenuation:

```cpp
attenuatedVolts = inputVolts * attenuation;
```

It then selects a scale from the resulting polarity:

```cpp
outputVolts = attenuatedVolts >= 0.0F
    ? attenuatedVolts * positiveScale
    : attenuatedVolts * negativeScale;
```

The complete intermediate state is returned in `AttenuverterResult`, which is
useful for debugging and later UI extensions.

## 7. Output limiting

`CvOutput` receives the calculated voltage.

Its snapshot distinguishes:

```text
requestedVolts
effectiveVolts
clampedLow
clampedHigh
```

Only `effectiveVolts` is sent toward the DAC and LED.

## 8. DAC conversion

The assumed output-stage transfer is reversed:

```text
Vdac = Vout / OUTPUT_SCALE + DAC_MIDPOINT_VOLTS
```

The result is normalized against the DAC reference, clamped to 0 through 1, and
converted to a 12-bit MCP4922 code.

## 9. LED output

The LED uses the same range-limited voltage request sent toward the DAC.

```text
positive -> green PWM
negative -> red PWM
near zero -> both off
```

Brightness is proportional to output magnitude after gamma shaping.

This is an indication of software intent. It is not feedback from the physical
output jack.

## 10. Error handling

If the SPI transfer succeeds, the LED is updated.

If it fails, `enterSafeState()`:

1. requests the nominal zero-volt DAC code
2. attempts another DAC transfer
3. switches both LED channels off

This remains an educational fallback rather than a complete fault-management
strategy.

## 11. Timing

The loop targets one update every 1000 microseconds.

The deadline comparison uses signed subtraction from the wrapping `micros()`
counter. The example does not yet count missed deadlines or adapt when execution
takes longer than one period.
