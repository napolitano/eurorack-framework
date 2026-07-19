/**
 * @file main.cpp
 * @brief Potentiometer minimal executable example.
 *
 * @details
 * Shows initialization, smoothing, deadband, and movement reporting.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/potentiometer.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::Potentiometer pot({0U, 1023U, false, 0.002F, 0.25F});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    pot.reset(0U);
    pot.update(700U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return pot.snapshot().changed ? 0 : 1;
}
