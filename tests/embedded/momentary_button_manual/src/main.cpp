/**
 * @file tests/embedded/momentary_button_manual/src/main.cpp
 * @brief Provides a human-operated embedded acceptance test for MomentaryButton.
 *
 * @details
 * The firmware guides a tester through release, press, hold, bounce, and release checks over the
 * serial monitor. It complements native unit tests by validating electrical wiring, pull-up
 * behavior, GPIO sampling, and real switch mechanics on an Arduino Nano compatible target.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @license PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <Arduino.h>
#include <eurorack/controls/momentary_button.hpp>

namespace {

constexpr std::uint8_t BUTTON_TEST_PIN = 2U;
constexpr std::uint32_t MINIMUM_HOLD_MS = 1000U;

eurorack::controls::MomentaryButton button({
    eurorack::controls::ActiveLevel::Low,
    20U,
});

bool pressObserved = false;
bool longHoldObserved = false;
bool releaseObserved = false;

void printResult(const __FlashStringHelper* name, const bool passed) {
    Serial.print(name);
    Serial.print(F(": "));
    Serial.println(passed ? F("PASS") : F("PENDING"));
}

void printSummary() {
    Serial.println(F("--- Manual acceptance status ---"));
    printResult(F("Stable press event"), pressObserved);
    printResult(F("Hold duration >= 1000 ms"), longHoldObserved);
    printResult(F("Stable release event"), releaseObserved);
    Serial.println(F("--------------------------------"));
}

} // namespace

void setup() {
    pinMode(BUTTON_TEST_PIN, INPUT_PULLUP);
    Serial.begin(115200);

    button.reset(digitalRead(BUTTON_TEST_PIN) == HIGH, millis());

    Serial.println(F("MomentaryButton manual hardware acceptance test"));
    Serial.println(F("1. Leave the button released."));
    Serial.println(F("2. Press and hold it for at least one second."));
    Serial.println(F("3. Release it."));
    Serial.println(F("Only one press and one release event should be reported."));
    printSummary();
}

void loop() {
    button.update(digitalRead(BUTTON_TEST_PIN) == HIGH, millis());
    const auto state = button.snapshot();

    if (state.justPressed) {
        pressObserved = true;
        Serial.println(F("Accepted press"));
        printSummary();
    }

    if (state.pressed && state.heldForMs >= MINIMUM_HOLD_MS) {
        longHoldObserved = true;
    }

    if (state.justReleased) {
        releaseObserved = true;
        Serial.print(F("Accepted release; completed press duration: "));
        Serial.print(state.lastPressDurationMs);
        Serial.println(F(" ms"));
        printSummary();
    }
}
