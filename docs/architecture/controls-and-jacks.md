# Controls and Jack Models

Step 3 adds generic models above the low-level hardware interfaces.

`Potentiometer` provides calibration, inversion, normalized deadband, smoothing,
and unipolar or bipolar values.

`EncoderValue` is deliberately separate from `RotaryEncoder`. The decoder
reports relative physical movement; the value model applies detents to bounded
parameters using clamp or wrap behavior.

The jack layer provides analog and digital input/output models for audio, CV,
pitch CV, gates, triggers, and clocks. Connection detection remains optional,
because many Eurorack sockets cannot reliably report whether a cable is present.

These models contain no Arduino, ADC, DAC, GPIO, comparator, or display API.
