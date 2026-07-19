# Project Tour

## Public headers

`include/eurorack/` contains the supported API.

- `controls/` - user-interface and signal-state models
- `core/` - project-wide configuration and foundational types
- `display/` - one-bit canvas, drawing, text, fonts, and widgets
- `drivers/` - concrete external IC and display drivers
- `io/` - abstract buses, pins, ADC, DAC, time, and jack contracts
- `platform/arduino/` - Arduino Core implementations of generic interfaces
- `simulation/` - deterministic native test doubles
- `storage/` - byte storage, checksums, records, and backends

## Implementations

Nontrivial implementations live in `src/`. Public declarations and behavior
contracts remain in `include/`.

## Tests

Native tests live in `tests/native/`. They exercise framework behavior without
physical hardware by using simulation backends.

## Documentation

- `docs/manual/` - newcomer-oriented handbook
- `docs/guides/` - focused procedures
- `docs/architecture/` - design decisions and subsystem boundaries
- `docs/release/` - release audits and known limitations
- inline Doxygen - API contracts and implementation rationale

## Tools

`tools/` contains repeatable test, formatting, documentation, audit, and release
scripts. Prefer these scripts over undocumented manual command sequences.
