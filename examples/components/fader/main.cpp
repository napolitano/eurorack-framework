/**
 * @file main.cpp
 * @brief Fader minimal executable example.
 *
 * @details
 * Shows fader direction, calibration endpoints, and normalized position.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/fader.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::Fader fader(
        {0U, 4095U, eurorack::controls::FaderDirection::BottomToTop, 0.001F, 1.0F});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    fader.reset(2048U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return fader.snapshot().normalized > 0.49F ? 0 : 1;
}
