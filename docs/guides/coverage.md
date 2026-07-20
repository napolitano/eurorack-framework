# Native coverage

The native coverage runner compiles the same granular dependency closure used by the native test runner and instruments it with GCC `gcov` line and branch counters.

## Commands

Run all native suites and enforce the general policy:

```bash
python tools/run-native-coverage.py
```

Run only the dedicated IC-driver suites and enforce the stricter driver policy:

```bash
python tools/run-native-coverage.py --driver-suites
```

Run one suite while developing:

```bash
python tools/run-native-coverage.py --filter test_driver_mcp4922 --no-threshold
```

Reports are written to:

- `build/coverage/coverage.json` for tools and CI;
- `docs/reference/native-coverage.md` for the human-readable summary.

## Policy

Thresholds are defined in `tools/coverage-policy.json`. Line and branch coverage are evaluated separately. Driver suites use the stricter `driverMinimum` values. The initial Alpha baseline reflects measured coverage rather than an aspirational number; thresholds may increase over time but must not silently decrease.

Coverage is a structural signal, not proof of correct hardware behavior. Protocol assertions, sanitizer runs, target builds and real-hardware tests remain separate quality gates.

## Release integration

The normal preflight omits coverage to keep its execution time bounded. Run the complete coverage gate explicitly:

```bash
python tools/release-check.py --coverage
```
