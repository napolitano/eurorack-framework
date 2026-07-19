# Control Behavior

## Attenuation control

The attenuation potentiometer is read as a normalized bipolar value.

Before the dead zone:

```text
ADC minimum   -> -1.0
ADC midpoint  ->  0.0
ADC maximum   -> +1.0
```

The example applies a centered dead zone:

```cpp
ATTENUATION_DEAD_ZONE = 0.035F;
```

A raw bipolar control value within -0.035 to +0.035 becomes exactly zero.

The remaining travel is rescaled. This avoids losing maximum gain:

```text
raw -1.000 -> processed -1.000
raw -0.035 -> processed  0.000
raw  0.000 -> processed  0.000
raw +0.035 -> processed  0.000
raw +1.000 -> processed +1.000
```

## Balance control

Balance uses the same centered dead-zone helper.

A positive balance value reduces only the negative side:

```text
positiveScale = 1.0
negativeScale = 1.0 - balance
```

A negative balance value reduces only the positive side:

```text
positiveScale = 1.0 + balance
negativeScale = 1.0
```

Because negative balance lies between -1 and 0, `1.0 + balance` lies between 0
and 1.

## Why balance is not an offset

A DC offset would use:

```text
output = attenuated + offset
```

That would move the electrical zero point and shift the complete waveform.

This example instead uses:

```text
output = attenuated * scale_for_current_polarity
```

Zero remains zero. Positive and negative ranges simply receive different gains.

## Processing order matters

Balance is applied after signed attenuation.

This means the balance control follows the polarity actually sent toward the
output. If attenuation is negative, the input polarity is inverted before
balance chooses a side.

That behavior is intentional because the LED and output stage should describe
the resulting signal, not the original input.

## LED mapping

The LED uses the range-limited output request.

```text
output > +25 mV -> green
output < -25 mV -> red
otherwise       -> off
```

Brightness is:

```text
normalized = abs(output) / 5 V
pwm = normalized ^ 1.8 * 255
```

The gamma exponent is a display choice. It does not alter the CV output.
