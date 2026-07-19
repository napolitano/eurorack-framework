/**
 * @file main.cpp
 * @brief DipSwitchBank minimal executable example.
 *
 * @details
 * Shows fixed-size switch storage and deterministic bit-mask packing.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <array>
#include <cstdint>
#include <eurorack/controls/dip_switch.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::DipSwitchBank<4U> bank({
        eurorack::controls::ActiveLevel::High, 0U});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    bank.reset({true, false, true, false}, 0U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return bank.bitMask() == 0x05U ? 0 : 1;
}
