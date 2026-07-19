/**
 * @file main.cpp
 * @brief CV input and output minimal executable example.
 *
 * @details
 * Shows voltage-domain input normalization and output range limiting.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <eurorack/controls/cv.hpp>
#include <eurorack/core/framework_config.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::CvInput input({-5.0F, 5.0F});
    eurorack::controls::CvOutput output({-5.0F, 5.0F});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    input.update(2.5F);
    output.setVolts(-2.5F);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return input.snapshot().normalized > 0.7F && output.snapshot().effectiveVolts < 0.0F ? 0 : 1;
}
