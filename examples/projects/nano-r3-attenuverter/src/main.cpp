/**
 * @file main.cpp
 * @brief Arduino Nano R3 one-channel attenuverter with side balance and bipolar LED.
 *
 * @details
 * The example reads:
 *
 * - one hypothetically protected and level-shifted bipolar CV input on A0
 * - one center-detent attenuation potentiometer on A1
 * - one center-detent balance potentiometer on A2
 *
 * The attenuation control produces a signed gain from -1.0 through zero to
 * +1.0. The balance control independently reduces either the positive or
 * negative side of the attenuated signal. The result is written to MCP4922
 * channel A and visualized by a red/green LED.
 *
 * This project demonstrates software structure only. The analog input stage,
 * DAC reference, op-amp output stage, jacks, protection, and LED resistors are
 * hypothetical and described in the accompanying Markdown files.
 */

#include "attenuverter_model.hpp"

#include <Arduino.h>
#include <SPI.h>
#if defined(__AVR__)
#include <eurorack/compat/avr/cmath.hpp>
#else
#include <cmath>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/controls/analog_input.hpp>
#include <eurorack/controls/cv.hpp>
#include <eurorack/drivers/dac/mcp4922.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/platform/arduino/arduino_gpio.hpp>
#include <eurorack/platform/arduino/arduino_spi.hpp>

namespace {

constexpr std::uint8_t CV_INPUT_PIN = A0;
constexpr std::uint8_t ATTENUATION_POT_PIN = A1;
constexpr std::uint8_t BALANCE_POT_PIN = A2;

constexpr std::uint8_t LED_RED_PWM_PIN = 5U;
constexpr std::uint8_t LED_GREEN_PWM_PIN = 6U;
constexpr std::uint8_t DAC_CHIP_SELECT_PIN = 10U;

constexpr std::uint32_t ADC_MAX_CODE = 1023U;
constexpr std::uint16_t DAC_MAX_CODE = 4095U;
constexpr std::uint8_t PWM_MAX_CODE = 255U;

constexpr float INPUT_MIN_VOLTS = -5.0F;
constexpr float INPUT_MAX_VOLTS = 5.0F;
constexpr float OUTPUT_MIN_VOLTS = -5.0F;
constexpr float OUTPUT_MAX_VOLTS = 5.0F;

/*
 * Center dead zones compensate for mechanical center-detent error and ADC noise.
 *
 * 0.035 corresponds to 3.5 percent of the bipolar control range. Values outside
 * the dead zone are rescaled so full travel still reaches exactly -1 and +1.
 */
constexpr float ATTENUATION_DEAD_ZONE = 0.035F;
constexpr float BALANCE_DEAD_ZONE = 0.035F;

/*
 * LED display behavior.
 *
 * The LED remains off close to zero output. Above the threshold, brightness is
 * proportional to the absolute output voltage. Positive output uses green;
 * negative output uses red.
 */
constexpr float LED_ZERO_THRESHOLD_VOLTS = 0.025F;
constexpr float LED_FULL_SCALE_VOLTS = 5.0F;
constexpr float LED_GAMMA = 1.8F;

/*
 * Hypothetical analog-stage calibration.
 *
 * Input:
 *     Vadc = INPUT_SCALE * Vin + INPUT_OFFSET_VOLTS
 *
 * Output:
 *     Vout = OUTPUT_SCALE * (Vdac - DAC_MIDPOINT_VOLTS)
 *
 * These constants describe the assumed circuit, not hardware included with the
 * example.
 */
constexpr float ADC_REFERENCE_VOLTS = 5.0F;
constexpr float INPUT_SCALE = 0.5F;
constexpr float INPUT_OFFSET_VOLTS = 2.5F;

constexpr float DAC_REFERENCE_VOLTS = 4.096F;
constexpr float DAC_MIDPOINT_VOLTS = 2.048F;
constexpr float OUTPUT_SCALE = 2.44140625F;

constexpr std::uint8_t OVERSAMPLE_COUNT = 8U;
constexpr std::uint32_t LOOP_PERIOD_US = 1000U;

eurorack::platform::arduino::ArduinoSpiBus spiBus(SPI);
eurorack::platform::arduino::ArduinoDigitalOutput dacChipSelect(DAC_CHIP_SELECT_PIN, true, false);

eurorack::drivers::dac::Mcp4922 dac(spiBus, dacChipSelect);

/*
 * The general AnalogInput model turns each 10-bit potentiometer reading into a
 * normalized and bipolar value. Dead-zone behavior remains application-specific
 * and is applied afterwards.
 */
eurorack::controls::AnalogInput attenuationControl({0U, ADC_MAX_CODE, false});
eurorack::controls::AnalogInput balanceControl({0U, ADC_MAX_CODE, false});

eurorack::controls::CvInput cvInput({INPUT_MIN_VOLTS, INPUT_MAX_VOLTS});
eurorack::controls::CvOutput cvOutput({OUTPUT_MIN_VOLTS, OUTPUT_MAX_VOLTS});

std::uint32_t nextUpdateUs = 0U;

/**
 * @brief Reads one ADC channel with a small arithmetic average.
 *
 * @param pin Arduino analog pin.
 * @return Averaged 10-bit ADC code.
 */
std::uint16_t readAveragedAdc(const std::uint8_t pin) noexcept {
    std::uint32_t sum = 0U;

    for (std::uint8_t index = 0U; index < OVERSAMPLE_COUNT; ++index) {
        sum += static_cast<std::uint32_t>(analogRead(pin));
    }

    return static_cast<std::uint16_t>(sum / OVERSAMPLE_COUNT);
}

/**
 * @brief Converts the assumed conditioned ADC voltage back into jack voltage.
 *
 * @param code Averaged ADC code.
 * @return Calibrated bipolar input voltage.
 */
float adcCodeToInputVolts(const std::uint16_t code) noexcept {
    const float adcVolts =
        static_cast<float>(code) * ADC_REFERENCE_VOLTS / static_cast<float>(ADC_MAX_CODE);

    return (adcVolts - INPUT_OFFSET_VOLTS) / INPUT_SCALE;
}

/**
 * @brief Converts a requested bipolar jack voltage into one MCP4922 code.
 *
 * @param outputVolts Requested voltage after the hypothetical op-amp stage.
 * @return Clamped 12-bit DAC code.
 */
std::uint16_t outputVoltsToDacCode(const float outputVolts) noexcept {
    const float requiredDacVolts = outputVolts / OUTPUT_SCALE + DAC_MIDPOINT_VOLTS;

    const float normalized =
        attenuverter_example::clampValue(requiredDacVolts / DAC_REFERENCE_VOLTS, 0.0F, 1.0F);

    return static_cast<std::uint16_t>(std::lround(normalized * static_cast<float>(DAC_MAX_CODE)));
}

/**
 * @brief Applies perceptual brightness shaping and returns one PWM code.
 *
 * @param magnitudeVolts Absolute output voltage.
 * @return Gamma-shaped PWM value from zero through 255.
 */
std::uint8_t outputMagnitudeToLedPwm(const float magnitudeVolts) noexcept {
    if (magnitudeVolts <= LED_ZERO_THRESHOLD_VOLTS) {
        return 0U;
    }

    const float normalized =
        attenuverter_example::clampValue(magnitudeVolts / LED_FULL_SCALE_VOLTS, 0.0F, 1.0F);

    const float perceptual = std::pow(normalized, LED_GAMMA);

    return static_cast<std::uint8_t>(std::lround(perceptual * static_cast<float>(PWM_MAX_CODE)));
}

/**
 * @brief Updates the hypothetical common-cathode red/green LED.
 *
 * @details
 * Positive output is green, negative output is red, and near-zero output is
 * dark. Only one die is active at a time.
 *
 * @param outputVolts Effective requested output voltage.
 */
void updateBipolarLed(const float outputVolts) noexcept {
    const std::uint8_t brightness = outputMagnitudeToLedPwm(std::fabs(outputVolts));

    if (outputVolts > LED_ZERO_THRESHOLD_VOLTS) {
        analogWrite(LED_RED_PWM_PIN, 0);
        analogWrite(LED_GREEN_PWM_PIN, brightness);
    } else if (outputVolts < -LED_ZERO_THRESHOLD_VOLTS) {
        analogWrite(LED_RED_PWM_PIN, brightness);
        analogWrite(LED_GREEN_PWM_PIN, 0);
    } else {
        analogWrite(LED_RED_PWM_PIN, 0);
        analogWrite(LED_GREEN_PWM_PIN, 0);
    }
}

/**
 * @brief Writes the nominal zero-volt code and extinguishes the LED.
 */
void enterSafeState() noexcept {
    dac.setCode(eurorack::drivers::dac::Mcp4922Channel::A, outputVoltsToDacCode(0.0F));

    static_cast<void>(dac.flushChannel(eurorack::drivers::dac::Mcp4922Channel::A));

    updateBipolarLed(0.0F);
}

} // namespace

void setup() {
    pinMode(CV_INPUT_PIN, INPUT);
    pinMode(ATTENUATION_POT_PIN, INPUT);
    pinMode(BALANCE_POT_PIN, INPUT);

    pinMode(LED_RED_PWM_PIN, OUTPUT);
    pinMode(LED_GREEN_PWM_PIN, OUTPUT);
    updateBipolarLed(0.0F);

    spiBus.begin();

    dac.setGain(eurorack::drivers::dac::Mcp4922Channel::A,
                eurorack::drivers::dac::Mcp4922Gain::OneX);
    dac.setEnabled(eurorack::drivers::dac::Mcp4922Channel::A, true);

    enterSafeState();
    nextUpdateUs = micros();
}

void loop() {
    const std::uint32_t nowUs = micros();

    /*
     * Signed subtraction is the conventional wrap-safe deadline test for a
     * periodically wrapping unsigned microsecond counter.
     */
    if (static_cast<std::int32_t>(nowUs - nextUpdateUs) < 0) {
        return;
    }
    nextUpdateUs += LOOP_PERIOD_US;

    const std::uint16_t cvCode = readAveragedAdc(CV_INPUT_PIN);
    const std::uint16_t attenuationCode = readAveragedAdc(ATTENUATION_POT_PIN);
    const std::uint16_t balanceCode = readAveragedAdc(BALANCE_POT_PIN);

    cvInput.update(adcCodeToInputVolts(cvCode));
    attenuationControl.update(attenuationCode);
    balanceControl.update(balanceCode);

    const float attenuation = attenuverter_example::applyCenteredDeadZone(
        attenuationControl.snapshot().bipolar, ATTENUATION_DEAD_ZONE);

    const float balance = attenuverter_example::applyCenteredDeadZone(
        balanceControl.snapshot().bipolar, BALANCE_DEAD_ZONE);

    const attenuverter_example::AttenuverterResult processed =
        attenuverter_example::process(cvInput.snapshot().volts, attenuation, balance);

    cvOutput.setVolts(processed.outputVolts);

    dac.setCode(eurorack::drivers::dac::Mcp4922Channel::A,
                outputVoltsToDacCode(cvOutput.snapshot().effectiveVolts));

    const eurorack::io::IoResult result =
        dac.flushChannel(eurorack::drivers::dac::Mcp4922Channel::A);

    if (result == eurorack::io::IoResult::Success) {
        updateBipolarLed(cvOutput.snapshot().effectiveVolts);
    } else {
        enterSafeState();
    }
}
