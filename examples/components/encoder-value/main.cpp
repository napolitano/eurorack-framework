/**
 * @file main.cpp
 * @brief EncoderValue minimal executable example.
 *
 * @details
 * Shows how signed detents update a bounded application value.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/encoder_value.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::EncoderValue value({
        0, 10, 1, eurorack::controls::EncoderBoundaryMode::Clamp}, 5);
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    value.applyDetents(3);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return value.snapshot().value == 8 ? 0 : 1;
}
