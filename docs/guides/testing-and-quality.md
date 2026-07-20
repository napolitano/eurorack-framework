# Testing and Quality Guide

Framework tests remain application-neutral. They verify state machines, data conversion, protocol frames, failure cleanup, allocation policy, dependency isolation, and platform contracts. They do not implement or validate a concrete Eurorack module.

## Test layers

1. Unit tests cover one model or policy.
2. Protocol tests assert exact SPI/I2C bytes and control-line cleanup.
3. Integration-contract tests combine generic buses, drivers, storage, and controls without musical behavior.
4. Compile tests build every example against only its manifest dependency closure.
5. Sanitizer tests detect native undefined behavior and leaks.
6. Embedded builds and size reports validate target-specific compilation and resource budgets.

Run the complete local preflight with `python tools/release-check.py`. AVR hardware qualification remains a separate documented activity.

## Native line coverage

With GCC and gcov installed, generate a repository report using:

```bash
python tools/run-native-coverage.py
```

Coverage is tracked as a diagnostic rather than a release-quality percentage target. Driver protocol branches and failure cleanup remain mandatory even when aggregate line coverage is high.
