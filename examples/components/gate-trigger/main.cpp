/**
 * @file main.cpp
 * @brief GateInput and TriggerOutput minimal executable example.
 *
 * @details
 * Shows hysteresis for gate detection and explicit trigger timing.
 *
 * This file intentionally uses literal samples. That keeps the example focused
 * on the framework state model. A consuming firmware project would obtain the
 * same values from platform adapters or hardware drivers.
 *
 * The final process exit code acts as a compact executable assertion:
 * zero means the expected state was observed.
 */

#include <cstdint>
#include <eurorack/controls/gate.hpp>

int main() {
    // 1. Construct the model with explicit settings. Defaults are avoided
    // here so the example also documents the meaning of each configuration.
    eurorack::controls::GateInput gate(1.0F, 2.0F);
    // 2. Supply one or more deterministic samples. State changes only
    // when the model receives an explicit method call.
    gate.update(2.5F);

    eurorack::controls::TriggerOutput trigger(10U);
    trigger.trigger(100U);
    trigger.update(111U);

    // 3. Inspect the immutable snapshot or focused accessor. Returning zero

    // makes the example usable in local scripts and CI.

    return gate.snapshot().high && !trigger.snapshot().high ? 0 : 1;
}
