/**
 * @file include/eurorack/platform/arduino/arduino_analog.hpp
 * @brief Declares Arduino analog input and PWM output adapters.
 *
 * @details
 * Wraps analogRead and analogWrite while keeping converter resolution explicit.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup platform_arduino
 */

#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include <cstdint>
#include <eurorack/io/analog_io.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino-backed analog input channel.
 */
class ArduinoAnalogInput final : public eurorack::io::AnalogInputChannel {
  public:
    /**
     * @brief Constructs an Arduino ADC channel.
     *
     * @param pin Analog-capable pin.
     * @param maximumCode Inclusive maximum ADC code.
     */
    ArduinoAnalogInput(std::uint8_t pin, std::uint32_t maximumCode = 1023U) noexcept
        : pin_(pin), maximumCode_(maximumCode) {}

    /**
     * @brief Reads one ADC conversion.
     *
     * @return Raw converter code and Success.
     */
    [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override {
        const int value = analogRead(pin_);
        if (value < 0) {
            return {0U, eurorack::io::IoResult::DataError};
        }

        const auto code = static_cast<std::uint32_t>(value);
        return {code <= maximumCode_ ? code : maximumCode_, eurorack::io::IoResult::Success};
    }

    /**
     * @brief Returns maximum ADC code.
     *
     * @return Inclusive maximum code.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override {
        return maximumCode_;
    }

  private:
    std::uint8_t pin_{0U};
    std::uint32_t maximumCode_{1023U};
};

/**
 * @brief Arduino analogWrite-backed PWM output.
 *
 * @details
 * This adapter represents PWM duty-cycle codes, not a true DAC voltage unless
 * the selected Arduino core maps analogWrite to a hardware DAC.
 */
class ArduinoPwmOutput final : public eurorack::io::AnalogOutputChannel {
  public:
    /**
     * @brief Constructs an Arduino PWM output.
     *
     * @param pin PWM-capable output pin.
     * @param maximumCode Inclusive PWM code.
     * @param initialCode Initial output code.
     */
    ArduinoPwmOutput(std::uint8_t pin,
                     std::uint32_t maximumCode = 255U,
                     std::uint32_t initialCode = 0U) noexcept
        : pin_(pin), maximumCode_(maximumCode) {
        pinMode(pin_, OUTPUT);
        static_cast<void>(writeRaw(initialCode <= maximumCode_ ? initialCode : maximumCode_));
    }

    /**
     * @brief Writes a PWM duty code.
     *
     * @param code Requested code.
     * @return Success or InvalidArgument.
     */
    eurorack::io::IoResult writeRaw(const std::uint32_t code) noexcept override {
        if (code > maximumCode_) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        analogWrite(pin_, static_cast<int>(code));
        lastCode_ = code;
        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Returns maximum PWM code.
     *
     * @return Inclusive maximum code.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override {
        return maximumCode_;
    }

    /**
     * @brief Returns last accepted output code.
     *
     * @return Last PWM duty code.
     */
    [[nodiscard]] std::uint32_t lastWrittenCode() const noexcept override {
        return lastCode_;
    }

  private:
    std::uint8_t pin_{0U};
    std::uint32_t maximumCode_{255U};
    std::uint32_t lastCode_{0U};
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO
