/**
 * @file include/eurorack/platform/arduino/arduino_gpio.hpp
 * @brief Declares Arduino digital pin adapters.
 *
 * @details
 * Wraps Arduino pinMode, digitalRead, and digitalWrite behind framework interfaces.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup platform_arduino
 */

#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/digital_io.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino-backed digital input.
 */
class ArduinoDigitalInput final : public eurorack::io::DigitalInput {
  public:
    /**
     * @brief Constructs and configures an Arduino digital input.
     *
     * @param pin Arduino pin number.
     * @param usePullup True to enable INPUT_PULLUP.
     * @param activeLow True to invert the logical reading.
     */
    ArduinoDigitalInput(std::uint8_t pin, bool usePullup = false, bool activeLow = false) noexcept
        : pin_(pin), activeLow_(activeLow) {
        pinMode(pin_, usePullup ? INPUT_PULLUP : INPUT);
    }

    /**
     * @brief Reads the logical input level.
     *
     * @return Logical high after optional active-low inversion.
     */
    [[nodiscard]] bool readHigh() const noexcept override {
        const bool physicalHigh = digitalRead(pin_) == HIGH;
        return activeLow_ ? !physicalHigh : physicalHigh;
    }

  private:
    std::uint8_t pin_{0U};
    bool activeLow_{false};
};

/**
 * @brief Arduino-backed digital output.
 */
class ArduinoDigitalOutput final : public eurorack::io::DigitalOutput {
  public:
    /**
     * @brief Constructs and configures an Arduino digital output.
     *
     * @param pin Arduino pin number.
     * @param initialHigh Initial logical level.
     * @param activeLow True to invert the physical output level.
     */
    ArduinoDigitalOutput(std::uint8_t pin,
                         bool initialHigh = false,
                         bool activeLow = false) noexcept
        : pin_(pin), activeLow_(activeLow), lastHigh_(initialHigh) {
        pinMode(pin_, OUTPUT);
        writeHigh(initialHigh);
    }

    /**
     * @brief Writes a logical output level.
     *
     * @param high Requested logical level.
     */
    void writeHigh(const bool high) noexcept override {
        lastHigh_ = high;
        const bool physicalHigh = activeLow_ ? !high : high;
        digitalWrite(pin_, physicalHigh ? HIGH : LOW);
    }

    /**
     * @brief Returns the last requested logical level.
     *
     * @return Last logical output level.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override {
        return lastHigh_;
    }

  private:
    std::uint8_t pin_{0U};
    bool activeLow_{false};
    bool lastHigh_{false};
};

/**
 * @brief Arduino-backed bidirectional digital pin.
 */
class ArduinoBidirectionalPin final : public eurorack::io::BidirectionalDigitalPin {
  public:
    /**
     * @brief Constructs a bidirectional Arduino pin.
     *
     * @param pin Arduino pin number.
     * @param direction Initial direction.
     * @param usePullup True to enable INPUT_PULLUP in input mode.
     * @param activeLow True to invert logical levels.
     */
    ArduinoBidirectionalPin(std::uint8_t pin,
                            Direction direction = Direction::Input,
                            bool usePullup = false,
                            bool activeLow = false) noexcept
        : pin_(pin), direction_(direction), usePullup_(usePullup), activeLow_(activeLow) {
        applyDirection();
    }

    /**
     * @brief Changes pin direction.
     *
     * @param direction Requested direction.
     */
    void setDirection(const Direction direction) noexcept override {
        direction_ = direction;
        applyDirection();
    }

    /**
     * @brief Returns current direction.
     *
     * @return Input or output direction.
     */
    [[nodiscard]] Direction direction() const noexcept override {
        return direction_;
    }

    /**
     * @brief Reads the logical pin state.
     *
     * @return Logical state after optional inversion.
     */
    [[nodiscard]] bool readHigh() const noexcept override {
        const bool physicalHigh = digitalRead(pin_) == HIGH;
        return activeLow_ ? !physicalHigh : physicalHigh;
    }

    /**
     * @brief Writes the logical output latch.
     *
     * @param high Requested logical level.
     */
    void writeHigh(const bool high) noexcept override {
        lastHigh_ = high;
        const bool physicalHigh = activeLow_ ? !high : high;
        digitalWrite(pin_, physicalHigh ? HIGH : LOW);
    }

    /**
     * @brief Returns the last requested logical level.
     *
     * @return Last logical output level.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override {
        return lastHigh_;
    }

  private:
    /**
     * @brief Applies current direction to Arduino pinMode.
     */
    void applyDirection() noexcept {
        if (direction_ == Direction::Output) {
            pinMode(pin_, OUTPUT);
            writeHigh(lastHigh_);
        } else {
            pinMode(pin_, usePullup_ ? INPUT_PULLUP : INPUT);
        }
    }

    std::uint8_t pin_{0U};
    Direction direction_{Direction::Input};
    bool usePullup_{false};
    bool activeLow_{false};
    bool lastHigh_{false};
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO
