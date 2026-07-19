# Component Examples

These examples are intentionally small. Each program demonstrates one public
control or signal model with literal input samples and verifies one result.

They do not pretend that raw MCU pins are Eurorack-safe interfaces.

Build every native component example:

```bash
python tools/build-examples.py
```

Build one example:

```bash
python tools/build-examples.py --example momentary-button
```

Available examples:

- `analog-input`
- `bicolor-led`
- `cv`
- `digital-led`
- `dip-switch`
- `encoder-value`
- `fader`
- `gate-trigger`
- `illuminated-button`
- `illuminated-fader`
- `momentary-button`
- `multicolor-led`
- `on-off-momentary-switch`
- `potentiometer`
- `rotary-encoder`
- `toggle-switch`

The complete Arduino Nano R3 attenuverter-with-balance application example is located at:

```text
examples/projects/nano-r3-attenuverter/
```
