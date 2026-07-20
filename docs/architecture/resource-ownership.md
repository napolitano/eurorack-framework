# Embedded Resource Ownership

Framework libraries are independently selectable, but hardware peripherals are not automatically composable. `framework-resources.json` records exclusive vectors, timers, pins, and shareable buses.

Validate a project profile with:

```bash
python tools/check-resource-conflicts.py examples/profiles/avr-shared-spi.json
```

SPI and I2C are marked shareable because correct adapters serialize transactions. Chip-select pins, I2C addresses, EEPROM address ranges, timer instances, and interrupt vectors remain application-level integration decisions.

The registry is intentionally conservative. A missing resource declaration is not proof that a library has no hardware requirements. New platform libraries must update the registry and CHANGELOG in the same commit.
