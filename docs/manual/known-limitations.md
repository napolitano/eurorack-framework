# Known Limitations

The framework remains a alpha development build.

Current limitations include:

- embedded validation is incomplete across every intended MCU core
- some simulator and native-storage components use dynamic allocation
- real-time suitability depends on the selected platform adapter and driver
- successful bus calls do not verify physical device behavior
- persistent formats may still change before a stable release
- documentation and examples are still expanding
- consuming firmware remains responsible for hardware protection and compliance

Review `docs/release/` for release-specific findings.
