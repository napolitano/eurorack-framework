# Continuous Integration

The repository uses three GitHub Actions workflows.

## Continuous Integration

`.github/workflows/ci.yml` runs for pushes, pull requests, and manual requests.

Required jobs:

- strict GCC native tests
- strict Clang native tests
- AddressSanitizer and UndefinedBehaviorSanitizer
- PlatformIO native integration tests
- public API documentation audit
- maintainability documentation audit
- Markdown documentation packaging
- clang-format verification
- cppcheck

The workflow uses read-only repository permissions and cancels obsolete runs for
the same branch.

## Documentation Build

`.github/workflows/documentation.yml` runs when documentation, public headers,
implementations, the Doxyfile, or the documentation build script changes.

It builds:

- the maintained Markdown documentation archive
- the combined Doxygen and LaTeX PDF manual

Both outputs are uploaded as workflow artifacts. The workflow does not publish a
website or GitHub Release.

## Development Artifacts

`.github/workflows/artifacts.yml` is manual because the project is Unreleased
Alpha. It packages source, Markdown documentation, checksums, and optionally the
PDF manual. The workflow uploads ephemeral Actions artifacts only.

It deliberately does not:

- create a Git tag
- publish a GitHub Release
- upload to a package registry
- describe the output as stable or release-qualified

## Recommended branch protection

Protect `main` and require these status checks:

- Native tests - g++
- Native tests - clang++
- Address and undefined-behavior sanitizers
- PlatformIO native integration
- Documentation audits
- Formatting and static analysis
- Markdown and PDF manual

Require pull requests and dismiss stale approvals after new commits. Do not
allow failed or skipped required checks to be bypassed for ordinary changes.

## Action maintenance

`.github/dependabot.yml` checks official GitHub Actions dependencies weekly.
Review major-version updates before merging because runner requirements and
artifact behavior can change.
