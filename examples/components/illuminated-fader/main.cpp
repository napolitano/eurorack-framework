/**
 * @file main.cpp
 * @brief IlluminatedFader minimal executable example.
 *
 * @details
 * Shows automatic LED brightness following normalized fader travel.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/illuminated_fader.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::IlluminatedFader fader(
        {{0U, 1023U, eurorack::controls::FaderDirection::BottomToTop, 0.0F, 1.0F},
         eurorack::controls::IlluminatedFaderMode::FollowPosition,
         {0U, 0U, 65535U},
         0U,
         65535U});
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    fader.reset(512U);
    // 3. Inspect the immutable snapshot or focused accessor. Returning zero
    // makes the example usable in local scripts and CI.
    return fader.led().snapshot().masterBrightness > 30000U ? 0 : 1;
}
