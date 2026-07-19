/**
 * @file examples/projects/nano-r3-oled-controller/src/main.cpp
 * @brief Reference firmware combining an OLED, an I2C GPIO expander, a push encoder, a bicolor
 * status LED, one CV input jack, one gate output jack, and an analog multiplexer on an Arduino
 * Nano R3 (ATmega328P).
 *
 * @details
 * This project demonstrates software structure only, in the same spirit as
 * `nano-r3-attenuverter`: it shows how the framework's controls, drivers, and platform adapters
 * compose into one firmware image and that the result fits an ATmega328P's flash and SRAM
 * budget. See `hardware.md` for the assumed wiring and its caveats.
 *
 * Loop structure: read the push encoder and its button, read one channel of the analog
 * multiplexer per iteration (round-robin across all channels rather than all at once, to keep
 * each `loop()` call short), read the CV input jack, update the gate output jack and status LED
 * from that state, and redraw the OLED.
 */

#include <Arduino.h>

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdio.hpp>
#else
#include <cstdio>
#endif

#include <eurorack/controls/bicolor_led.hpp>
#include <eurorack/controls/momentary_button.hpp>
#include <eurorack/controls/rotary_encoder.hpp>
#include <eurorack/display/monochrome_canvas.hpp>
#include <eurorack/display/widgets.hpp>
#include <eurorack/drivers/display/ssd1306.hpp>
#include <eurorack/drivers/gpio/mcp23017.hpp>
#include <eurorack/drivers/mux/analog_multiplexer.hpp>
#include <eurorack/io/jack.hpp>
#include <eurorack/platform/arduino/arduino_analog.hpp>
#include <eurorack/platform/arduino/arduino_gpio.hpp>
#include <eurorack/platform/arduino/arduino_i2c.hpp>
#include <eurorack/platform/arduino/arduino_time.hpp>

namespace {

// --- Pin assignment -------------------------------------------------------
// I2C (fixed by hardware): A4 = SDA, A5 = SCL. Shared by the OLED (0x3C) and the MCP23017
// (0x20, all three address pins tied low).
constexpr std::uint8_t ENCODER_PIN_A = 2U; // Interrupt-capable; polled here, not ISR-driven.
constexpr std::uint8_t ENCODER_PIN_B = 3U; // Interrupt-capable; polled here, not ISR-driven.
constexpr std::uint8_t ENCODER_BUTTON_PIN = 4U;
constexpr std::uint8_t MUX_SELECT_PIN_0 = 5U; // 74HC4051 select line S0 (address bit 0).
constexpr std::uint8_t MUX_SELECT_PIN_1 = 6U; // 74HC4051 select line S1 (address bit 1).
constexpr std::uint8_t MUX_SELECT_PIN_2 = 7U; // 74HC4051 select line S2 (address bit 2).
constexpr std::uint8_t MUX_COMMON_ADC_PIN = A0;
constexpr std::uint8_t CV_INPUT_ADC_PIN = A1;
constexpr std::size_t MUX_CHANNELS_USED = 4U; // Only the first 4 of 8 possible mux channels.
constexpr std::size_t MCP_GATE_OUTPUT_PIN = 0U;
constexpr std::size_t MCP_LED_RED_PIN = 1U;
constexpr std::size_t MCP_LED_GREEN_PIN = 2U;

// --- Platform adapters -----------------------------------------------------
eurorack::platform::arduino::ArduinoTime g_time;
eurorack::platform::arduino::ArduinoI2cBus g_i2c;
eurorack::platform::arduino::ArduinoAnalogInput g_muxCommonInput(MUX_COMMON_ADC_PIN);
eurorack::platform::arduino::ArduinoAnalogInput g_cvInputChannel(CV_INPUT_ADC_PIN);
eurorack::platform::arduino::ArduinoDigitalOutput g_muxSelect0(MUX_SELECT_PIN_0);
eurorack::platform::arduino::ArduinoDigitalOutput g_muxSelect1(MUX_SELECT_PIN_1);
eurorack::platform::arduino::ArduinoDigitalOutput g_muxSelect2(MUX_SELECT_PIN_2);

// --- Drivers ---------------------------------------------------------------
eurorack::drivers::display::Ssd1306 g_oled(g_i2c);
eurorack::drivers::gpio::Mcp23017 g_expander(g_i2c, eurorack::io::I2cAddress{0x20U});
eurorack::drivers::mux::AnalogMultiplexer<3>
    g_mux({&g_muxSelect0, &g_muxSelect1, &g_muxSelect2}, g_muxCommonInput, g_time, 10U);

// --- Controls and jacks ------------------------------------------------------
eurorack::controls::RotaryEncoder g_encoder;
eurorack::controls::MomentaryButton g_encoderButton;
eurorack::controls::BiColorLed g_statusLed;
eurorack::io::AnalogInputJack g_cvInputJack{eurorack::io::JackSignalType::Cv};
eurorack::io::DigitalOutputJack g_gateOutputJack{eurorack::io::JackSignalType::Gate};

// --- Display -----------------------------------------------------------------
constexpr std::int32_t DISPLAY_WIDTH = 128;
constexpr std::int32_t DISPLAY_HEIGHT = 64;
std::uint8_t g_canvasBuffer[static_cast<std::size_t>(DISPLAY_WIDTH) * DISPLAY_HEIGHT / 8U];
eurorack::display::MonochromeCanvas
    g_canvas(g_canvasBuffer, sizeof(g_canvasBuffer), DISPLAY_WIDTH, DISPLAY_HEIGHT);

std::size_t g_muxChannel = 0U;
std::uint32_t g_muxChannelCodes[MUX_CHANNELS_USED] = {0U, 0U, 0U, 0U};

void applyStatusLed() noexcept {
    if (g_gateOutputJack.snapshot().high) {
        g_statusLed.setColor(eurorack::controls::BiColorLedColor::ColorB);
    } else {
        g_statusLed.setColor(eurorack::controls::BiColorLedColor::ColorA);
    }

    const eurorack::controls::BiColorLedSnapshot& led = g_statusLed.snapshot();
    static_cast<void>(
        g_expander.writePin(MCP_LED_RED_PIN, led.pinA == eurorack::controls::PinDrive::High));
    static_cast<void>(
        g_expander.writePin(MCP_LED_GREEN_PIN, led.pinB == eurorack::controls::PinDrive::High));
}

void redrawDisplay() noexcept {
    const eurorack::controls::RotaryEncoderSnapshot& encoderState = g_encoder.snapshot();

    char valueText[8];
    static_cast<void>(std::snprintf(
        valueText, sizeof(valueText), "%ld", static_cast<long>(encoderState.position)));

    eurorack::display::drawParameter(
        g_canvas,
        {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT},
        {"ENCODER", valueText, "", g_encoderButton.snapshot().pressed});

    static_cast<void>(g_oled.flush(g_canvas));
}

} // namespace

void setup() {
    Wire.begin();

    static_cast<void>(g_oled.initialize());

    static_cast<void>(g_expander.initialize());
    // Pin 0 (gate output) and pins 1-2 (bicolor LED) are outputs; the remaining 13 pins are
    // left as inputs with pull-ups enabled for future expansion (spare jacks, panel switches).
    static_cast<void>(g_expander.setDirection(0xFFF8U));
    static_cast<void>(g_expander.setPullUps(0xFFF8U));

    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

    g_encoder.reset(digitalRead(ENCODER_PIN_A) == HIGH, digitalRead(ENCODER_PIN_B) == HIGH, 0);
}

void loop() {
    g_encoder.update(digitalRead(ENCODER_PIN_A) == HIGH, digitalRead(ENCODER_PIN_B) == HIGH);
    g_encoderButton.update(digitalRead(ENCODER_BUTTON_PIN) == HIGH, g_time.milliseconds());

    // Round-robin one multiplexer channel per loop iteration instead of reading all of them
    // back-to-back, so a single loop() call stays short.
    g_muxChannelCodes[g_muxChannel] = g_mux.readChannel(g_muxChannel).code;
    g_muxChannel = (g_muxChannel + 1U) % MUX_CHANNELS_USED;

    g_cvInputJack.update(g_cvInputChannel.readRaw().code * (5.0F / 1023.0F));
    g_gateOutputJack.setHigh(g_encoderButton.snapshot().pressed);

    static_cast<void>(g_expander.writePin(MCP_GATE_OUTPUT_PIN, g_gateOutputJack.snapshot().high));
    applyStatusLed();

    redrawDisplay();
}
