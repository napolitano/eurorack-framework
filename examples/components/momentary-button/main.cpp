/**
 * @file main.cpp
 * @brief MomentaryButton minimal executable example.
 *
 * @details
 * Shows active-low conversion and timestamp-based debounce acceptance.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/momentary_button.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::MomentaryButton button({eurorack::controls::ActiveLevel::Low, 10U});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    button.reset(true, 0U);
    button.update(false, 1U);
    button.update(false, 11U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return button.wasPressed() ? 0 : 1;
}
