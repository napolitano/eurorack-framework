# Root PlatformIO workflow

## Purpose

The repository root is a native PlatformIO development project. It provides a fast smoke build and
access to all native Unity tests without requiring a consuming firmware repository.

## Build

Run from the directory containing `platformio.ini`:

```bash
pio run
```

PlatformIO builds `src/main.cpp` with the `native` environment. The executable checks framework
configuration and a fixed-capacity event queue. This verifies root configuration and local granular
library discovery. It is not an embedded compatibility claim.

## Tests

```bash
pio test -e native
```

PlatformIO discovers suites below `tests/native/`. `test_build_src = false` is intentional: each
Unity suite owns its entry point, while `src/main.cpp` belongs only to the smoke build.

A single suite can be selected with PlatformIO's filter:

```bash
pio test -e native -f test_driver_mcp4922
```

## Cleaning

```bash
pio run -t clean
```

Removing `.pio/` is safe if a complete dependency/platform refresh is required.

## VSCodium

Open the repository root, not the `libraries/` directory. The PlatformIO extension should show the
`native` environment under Project Tasks. Select Build or Test there.

## Troubleshooting

Confirm PlatformIO and the project configuration:

```bash
pio --version
pio project config
pio platform list
```

Install the native host platform when necessary:

```bash
pio platform install native
```

The separate Python runners remain authoritative for granular dependency-closure checks, sanitizer
runs, and coverage. They complement rather than replace the ergonomic root PlatformIO workflow.
