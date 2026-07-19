# Conceptual Troubleshooting

## Center attenuation does not mute

Possible causes:

- center code lies outside the configured dead zone
- potentiometer detent is not electrically centered
- ADC noise is larger than expected
- endpoint calibration is asymmetric

Increase the dead zone only after inspecting raw A1 readings.

## Center balance is not symmetric

Inspect A2 raw code at the detent. A piecewise center calibration may be more
appropriate than a symmetric dead zone.

## Balance appears reversed

The example defines:

```text
left  -> negative side favored
right -> positive side favored
```

Reverse the A2 wiring or set the control model to inverted if the panel direction
should be opposite.

## Balance changes the wrong polarity after inversion

Remember that balance follows the attenuated output polarity. With negative
attenuation, original input polarity is reversed before balance is applied.

This is intentional.

## LED polarity is reversed

Check:

- red and green pin assignment
- common-cathode versus common-anode package
- LED pinout
- PWM inversion requirements

The code assumes common cathode.

## LED is dark at small voltages

The example deliberately uses:

```cpp
LED_ZERO_THRESHOLD_VOLTS
LED_GAMMA
```

Lower the threshold or gamma only if the resulting indication is useful.

## Output arithmetic looks right but hardware would not

The code assumes specific input and output transfer functions. A hypothetical
or future circuit must actually implement them.

Software cannot compensate for an undefined analog path.
