# GitHub Actions and local CI

The repository uses separate workflows for builds, tests, quality audits,
documentation, and manually requested development artifacts. All workflows use
read-only repository permissions and explicit Ubuntu runner versions.

## Workflow responsibilities

| Workflow | Responsibility |
|---|---|
| `build.yml` | PlatformIO native integration and Arduino Nano R3 example builds |
| `tests.yml` | Native tests with GCC and Clang, per-driver sanitizers, component examples |
| `ci.yml` | Documentation, licensing, formatting, and static-analysis audits |
| `documentation.yml` | Markdown and PDF manual generation with diagnostics |
| `artifacts.yml` | Manually requested deterministic development artifacts |

Sanitizer jobs are split by driver suite. This avoids one oversized process,
keeps failures attributable to one driver, and permits GitHub Actions to execute
independent suites in parallel.

## Local equivalents

Run the portable quality checks before pushing:

```bash
python tools/check-github-actions.py
python tools/check-format.py
python tools/check-library-layout.py
python tools/build-examples.py
python tools/run-native-tests.py
```

Run optional host tooling when installed:

```bash
python tools/check-static-analysis.py
python tools/run-native-tests.py --driver-suites --sanitizers
python tools/run-native-coverage.py --driver-suites
```

The complete configurable preflight is:

```bash
python tools/release-check.py \
  --static-analysis \
  --sanitizers \
  --coverage \
  --archive
```

## Workflow validation

`tools/check-github-actions.py` provides a dependency-free structural guard. It
checks that every workflow has the required top-level sections, uses explicit
runner images, uses the project-approved major versions of official actions,
does not use `pull_request_target`, and does not silently request write access.

This is not a full YAML or GitHub Actions interpreter. GitHub remains the final
validator of workflow semantics. The local check is intended to catch common
maintenance mistakes before a push.

## Static analysis

The quality workflow installs `clang-format` and `cppcheck` from the Ubuntu
package repositories. Include directories are discovered down to the actual
per-library `include` directories. The repository's Python static-analysis tool
continues to perform strict compiler checks and public-header self-containment
checks locally and in release preparation.

## Updating action versions

Official action major versions are centralized as policy in
`tools/check-github-actions.py`. Update workflow references and that policy in
the same change. Review the official migration notes before accepting a new
major version because official actions may change their Node runtime or artifact
behavior.
