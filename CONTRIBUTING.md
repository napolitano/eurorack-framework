# Contributing

Contributions are welcome when they fit the scope, architecture, documentation standards, and licensing model of Eurorack Framework.

## Before contributing

Open an issue before starting a substantial feature, new platform backend, public API change, or new peripheral family. Small corrections and documentation improvements may be submitted directly.

## Requirements

Contributions must:

- be original work that you are permitted to license to the project;
- be compatible with the repository licensing model;
- avoid copying code from incompatible or undisclosed sources;
- include documentation for public APIs;
- include tests where the behavior can be tested without hardware;
- include an embedded validation example where hardware interaction is central;
- avoid dynamic allocation in core components unless explicitly justified;
- preserve deterministic behavior and bounded resource use;
- use US English in code comments and documentation.

## Style

- C++17 is the baseline language version.
- Public APIs use descriptive domain terminology.
- Platform-specific types do not leak into portable component APIs.
- Each source and header file must include the project file header.
- Public functions document parameters, return values, ranges, units, timing, ownership, and interrupt-safety expectations.
- Warnings enabled in `platformio.ini` should remain clean.

## Licensing contributions

By submitting a contribution, you certify that you have the right to contribute it and agree that it may be distributed under the repository's license terms.

Commercial licensing remains controlled by the project maintainer. Contributions do not grant contributors rights to issue commercial licenses for the complete project unless separately agreed in writing.

## Doxygen documentation standard

Every class, struct, enum, method, function, constructor, and non-obvious data
member in framework code must be documented with Doxygen.

Every method or function must include:

- a precise `@brief`;
- one `@param` entry for every named parameter;
- an `@return` entry whenever the return type is not `void`;
- relevant units, valid ranges, ownership, lifetime, state changes, side
  effects, timing assumptions, and event lifetime;
- explicit statements when a function performs no hardware access or has no
  side effects, where that distinction matters.

Private helpers are not exempt. Doxygen is configured with
`EXTRACT_PRIVATE = YES`, `WARN_IF_UNDOCUMENTED = YES`,
`WARN_NO_PARAMDOC = YES`, and `WARN_AS_ERROR = YES`. Documentation warnings are
therefore build failures and must not be suppressed.
