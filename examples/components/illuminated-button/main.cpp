/**
 * @file main.cpp
 * @brief IlluminatedButton minimal executable example.
 *
 * @details
 * Shows automatic RGB feedback following a debounced button state.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/illuminated_button.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::IlluminatedButton button({
        {eurorack::controls::ActiveLevel::High, 0U},
        eurorack::controls::IlluminatedButtonMode::LitWhilePressed,
        {0U, 65535U, 0U},
        {0U, 0U, 0U}});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    button.reset(false, 0U);
    button.update(true, 1U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return button.led().snapshot().effective.green == 65535U ? 0 : 1;
}
