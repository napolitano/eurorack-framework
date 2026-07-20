# Quality tooling

The repository provides portable quality gates that can run on Windows, Linux, and macOS. Target builds remain separate because AVR and Arduino platform headers require the corresponding toolchain.

## Source hygiene

```bash
python tools/check-source-hygiene.py
```

Rejects temporary files, Python caches, editor backups, object files, and generated build debris from the source tree.

## Formatting

```bash
python tools/check-format.py
```

Always checks UTF-8 encoding, LF line endings, final newlines, and trailing whitespace. When `clang-format` is installed, it additionally runs `.clang-format` in dry-run error mode. CI environments that require the external formatter can use `--require-clang-format`.

## Static analysis

```bash
python tools/check-static-analysis.py
```

The portable baseline compiles every platform-independent implementation with strict warnings and verifies that each public header is self-contained. When available, `cppcheck` and `clang-tidy` run automatically. Use `--require-external` where their installation is mandatory.

AVR- and Arduino-specific libraries are deliberately excluded from the host analysis and must be checked by the target build.

## Sanitizers

```bash
python tools/run-native-tests.py --driver-suites --sanitizers
```

Runs the dedicated driver suites with AddressSanitizer and UndefinedBehaviorSanitizer. Individual suites can be selected with `--filter` to reduce iteration time.

## Release archive validation

```bash
python tools/validate-release-archive.py path/to/source.zip
```

Checks deterministic timestamps and ordering, required files, UTF-8 text files, a single archive root, and exclusion of build or temporary artifacts. `build-artifacts.py` invokes this validation automatically after creating the source archive.

## Combined release check

```bash
python tools/release-check.py --static-analysis --sanitizers --coverage --archive
```

The optional flags make expensive checks explicit. The default preflight still runs source hygiene and portable formatting checks.
