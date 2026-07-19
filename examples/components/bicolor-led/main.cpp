/**
 * @file main.cpp
 * @brief BiColorLed minimal executable example.
 *
 * @details
 * Shows topology-aware translation from a requested color to effective state.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <eurorack/controls/bicolor_led.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::BiColorLed led({
        eurorack::controls::BiColorLedTopology::CommonCathode,
        eurorack::controls::BiColorLedColor::Off});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    led.setColor(eurorack::controls::BiColorLedColor::ColorA);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return led.snapshot().effectiveColor ==
        eurorack::controls::BiColorLedColor::ColorA ? 0 : 1;
}
