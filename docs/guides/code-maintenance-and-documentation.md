# Code Maintenance and Documentation Standard

This project treats documentation as part of the implementation. A declaration
that merely repeats a symbol name is not considered documented.

## Public API documentation

Every public type and callable must explain:

- why the abstraction exists
- what state it owns and what it references without ownership
- valid units, ranges, polarity, and coordinate conventions
- when event flags are valid and when they are cleared
- whether the operation allocates, blocks, performs I/O, or mutates cached state
- what each error result means for object state
- behavior at timer, counter, and numeric wraparound
- requirements placed on the caller and referenced objects
- hardware behavior that is deliberately outside the abstraction

Every named parameter requires `@param`. Every non-void return requires
`@return`. Configuration fields and snapshot fields require member comments
that describe semantics, not only their C++ type.

## Implementation documentation

Implementation comments explain decisions that are not obvious from the
language syntax. Useful comments include:

- why a particular storage slot is selected
- why a byte order or controller frame layout is used
- why arithmetic is signed or unsigned
- where clipping, clamping, or wraparound is intentional
- why an intermediate buffer has a fixed size
- which operation is the hardware synchronization point
- how an invalid physical state is represented
- what remains true after a failed bus or storage operation

Comments must not narrate trivial syntax. For example, `increment counter` adds
no value next to `++counter`. Explain why that counter is incremented at that
specific state transition instead.

## State machines

Every state machine must document:

- initial state
- candidate or transitional state
- acceptance condition
- generated edge events
- lifetime of transient flags
- timer wraparound behavior
- handling of impossible inputs

## Hardware drivers

Every driver must document:

- bus mode, byte order, maximum clock assumptions, and transaction boundaries
- chip-select, latch, reset, and output-enable polarity
- cached values and whether failed writes roll them back
- controller-specific frame layouts
- dynamic-allocation and blocking behavior
- the distinction between accepted bus traffic and verified physical output

## Native-only components

Simulation and file-backed components may use the standard library and dynamic
allocation, but they must state this explicitly. Embedded headers and source
files must not accidentally depend on native-only facilities.

## Review procedure

Before accepting a change:

```bash
python tools/check-public-docs.py
python tools/check-maintainability-docs.py
python tools/run-native-tests.py --compiler g++
python tools/run-native-tests.py --compiler clang++
```

The documentation audits are intentionally conservative. Passing them does not
prove that prose is useful; reviewers must still reject comments that only
repeat names or types.

## Declarations own public Doxygen contracts

Public parameter and return contracts belong in declarations under `include/`.

Definitions under `src/` may contain ordinary implementation comments that
explain algorithms, invariants, hardware behavior, or non-obvious decisions.
They must not repeat `@param`, `@return`, or other API-contract sections already
attached to a declaration.

Doxygen merges documentation from declarations and definitions. Repeating the
same contract in both places can produce errors such as:

```text
argument raw has multiple @param documentation sections
```

Run the dedicated audit before building the manual:

```bash
python tools/check-doxygen-contracts.py
```

