#include <Arduino.h>
#include <eurorack/platform/arduino/arduino_platform.hpp>

eurorack::platform::arduino::ArduinoTime timeSource;
eurorack::platform::arduino::ArduinoDigitalOutput statusLed(LED_BUILTIN, false);
eurorack::platform::arduino::ArduinoAnalogInput controlInput(A0);

void setup() {}

void loop() {
    const auto sample = controlInput.readRaw();

    if (sample.result == eurorack::io::IoResult::Success) {
        const bool upperHalf = sample.code > controlInput.maximumCode() / 2U;
        statusLed.writeHigh(upperHalf);
    }

    timeSource.delayMicroseconds(1000U);
}
