# Momentary Button and Digital LED Example

## Hardware

Use a normally open momentary tactile switch. The example also uses the Arduino Nano's built-in LED.

Connect:

```text
Arduino Nano D2 ---- switch ---- GND
```

The example enables the microcontroller's internal pull-up resistor. The pin is
therefore high while the switch is released and low while pressed.

Do not connect an external voltage source to D2 for this example.

## Build and upload

From the example directory:

```bash
cd examples/momentary-button
pio run
pio run --target upload
pio device monitor
```

Pressing and releasing the switch should produce one stable press event and one
stable release event despite normal mechanical contact bounce. The built-in LED
must remain on only while the debounced button state is pressed.

The example keeps GPIO access outside both controls:

```cpp
button.update(rawLevelHigh, millis());
const auto buttonState = button.snapshot();

led.setOn(buttonState.pressed);
digitalWrite(LED_BUILTIN, led.rawOutputHigh() ? HIGH : LOW);
```

## Native tests

From the repository root:

```bash
pio test -e native
```

The native suite validates behavior without hardware.

## Manual embedded acceptance test

From the manual test directory:

```bash
cd tests/embedded/momentary_button_manual
pio run
pio run --target upload
pio device monitor
```

Follow the serial instructions. The test validates the target's GPIO wiring,
internal pull-up configuration, actual switch polarity, and real mechanical
behavior.
