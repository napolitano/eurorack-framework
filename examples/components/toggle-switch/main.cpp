/**
 * @file main.cpp
 * @brief ToggleSwitch minimal executable example.
 *
 * @details
 * Shows a maintained switch candidate becoming stable after debounce.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/toggle_switch.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::ToggleSwitch sw({eurorack::controls::ActiveLevel::High, 5U});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    sw.reset(false, 0U);
    sw.update(true, 1U);
    sw.update(true, 6U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return sw.isOn() ? 0 : 1;
}
