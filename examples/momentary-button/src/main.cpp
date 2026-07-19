/**
 * @file examples/momentary-button/src/main.cpp
 * @brief Demonstrates MomentaryButton on an Arduino Nano compatible board.
 *
 * @details
 * The example reads a normally open tactile switch connected between digital pin 2 and ground.
 * Arduino's internal pull-up resistor keeps the input high while released. Stable press and release
 * events are written to the serial monitor.
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
#include <eurorack/controls/digital_led.hpp>
#include <eurorack/controls/momentary_button.hpp>

namespace {

constexpr std::uint8_t BUTTON_DEMO_PIN = 2U;
constexpr std::uint8_t LED_DEMO_PIN = LED_BUILTIN;

eurorack::controls::MomentaryButton button({
    eurorack::controls::ActiveLevel::Low,
    20U,
});

eurorack::controls::DigitalLed led({
    eurorack::controls::LedActiveLevel::High,
    false,
});

} // namespace

void setup() {
    pinMode(BUTTON_DEMO_PIN, INPUT_PULLUP);
    pinMode(LED_DEMO_PIN, OUTPUT);
    Serial.begin(115200);

    const bool rawLevelHigh = digitalRead(BUTTON_DEMO_PIN) == HIGH;
    button.reset(rawLevelHigh, millis());
    led.reset();
    digitalWrite(LED_DEMO_PIN, led.rawOutputHigh() ? HIGH : LOW);

    Serial.println(F("Eurorack Framework - Momentary Button and LED Example"));
    Serial.println(F("Connect a normally open switch between D2 and GND."));
    Serial.println(F("The built-in LED remains on while the button is pressed."));
}

void loop() {
    const bool rawLevelHigh = digitalRead(BUTTON_DEMO_PIN) == HIGH;
    button.update(rawLevelHigh, millis());
    const auto state = button.snapshot();

    led.setOn(state.pressed);
    digitalWrite(LED_DEMO_PIN, led.rawOutputHigh() ? HIGH : LOW);

    if (state.justPressed) {
        Serial.println(F("BUTTON_DEMO pressed"));
    }

    if (state.justReleased) {
        Serial.print(F("BUTTON_DEMO released after "));
        Serial.print(state.lastPressDurationMs);
        Serial.println(F(" ms"));
    }
}
