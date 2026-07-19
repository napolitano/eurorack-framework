/**
 * @file main.cpp
 * @brief RotaryEncoder minimal executable example.
 *
 * @details
 * Shows one complete Gray-code cycle and a completed detent.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/rotary_encoder.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::RotaryEncoder encoder({4U, false});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    encoder.reset(false, false, 0);
    encoder.update(true, false);
    encoder.update(true, true);
    encoder.update(false, true);
    encoder.update(false, false);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return encoder.snapshot().delta != 0 ? 0 : 1;
}
