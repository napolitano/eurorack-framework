# Extended Panel Controls

The extended control layer adds models for illuminated buttons, maintained
switches, DIP-switch banks, On-Off-(On) switches, RGB LEDs, faders, and
illuminated faders.

Every control accepts already sampled electrical or ADC values. The models do
not own GPIO, ADC, PWM, or LED-driver interfaces. This keeps electrical setup,
current limiting, pull resistors, and platform APIs in their existing adapter
layers.

## Important electrical distinction

The RGB model stores logical channel brightness. It does not directly model a
common-anode or common-cathode package. A hardware adapter must map logical red,
green, and blue brightness to the actual driver topology and must provide
appropriate current limiting.

The On-Off-(On) model requires two contacts. Neither asserted represents center
Off. Both asserted is reported as Invalid rather than silently choosing one
side.
