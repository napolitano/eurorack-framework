/**
 * @file main.cpp
 * @brief AnalogInput minimal executable example.
 *
 * @details
 * Shows how calibrated raw ADC codes become normalized and bipolar state.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/analog_input.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::AnalogInput input({0U, 1023U, false});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    input.update(512U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return input.snapshot().normalized > 0.49F ? 0 : 1;
}
