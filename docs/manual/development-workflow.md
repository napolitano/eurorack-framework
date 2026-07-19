# Development Workflow

## Before editing

1. Read the relevant manual and architecture document.
2. Identify whether the change belongs in the framework or a consuming module.
3. Check object ownership, units, polarity, timing, and failure semantics.
4. Add or update tests before declaring the change complete.

## Required validation

```bash
python tools/check-public-docs.py
python tools/check-maintainability-docs.py
python tools/run-native-tests.py --compiler g++
python tools/run-native-tests.py --compiler clang++
```

PlatformIO integration:

```bash
pio run --target clean
pio test -e native
```

## Documentation expectations

Public comments must explain purpose, ownership, units, valid ranges, transient
state, errors, allocation, blocking, and hardware boundaries.

Implementation comments should explain non-obvious decisions. They should not
narrate trivial syntax.

## Release artifacts

```bash
python tools/build-artifacts.py
```

The script validates the project, packages source and Markdown documentation,
and builds a PDF when Doxygen and LaTeX are available.
