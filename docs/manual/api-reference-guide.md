# API Reference Guide

The generated PDF includes the API reference after the project manual.

Use the API reference when you already know which subsystem and type you need.
Use this manual and the architecture documents when you need to understand the
design first.

Each public API entry should state:

- purpose
- ownership and lifetime
- units and valid ranges
- transient versus persistent state
- error behavior
- allocation and blocking behavior
- thread or ISR assumptions
- hardware responsibilities left to the integrator

The API is grouped by Doxygen modules such as controls, I/O, drivers, storage,
display, simulation, and platform adapters.
