/**
 * @file include/eurorack/platform/arduino/arduino_eeprom_storage.hpp
 * @brief Declares an Arduino EEPROM persistent-storage adapter.
 *
 * @details
 * Maps the framework byte-storage contract to Arduino EEPROM while avoiding redundant writes
 * through EEPROM.update.
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
#include <EEPROM.h>
#include <cstddef>
#include <cstdint>
#include <eurorack/storage/persistent_storage.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino EEPROM-backed persistent storage.
 */
class ArduinoEepromStorage final : public eurorack::storage::PersistentStorage {
  public:
    /**
     * @brief Constructs an EEPROM storage view.
     *
     * @param baseAddress First exposed EEPROM byte.
     * @param capacityBytes Exposed byte count. Zero uses remaining EEPROM.
     * @param erasedValue Value used by erase operations.
     */
    ArduinoEepromStorage(std::size_t baseAddress = 0U,
                         std::size_t capacityBytes = 0U,
                         std::uint8_t erasedValue = 0xFFU) noexcept
        : baseAddress_(baseAddress),
          capacityBytes_(capacityBytes == 0U ? remainingCapacity(baseAddress)
                                             : boundedCapacity(baseAddress, capacityBytes)),
          erasedValue_(erasedValue) {}

    /**
     * @brief Begins emulated EEPROM on cores that require it.
     *
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult begin() noexcept {
#if defined(ESP8266) || defined(ESP32)
        return EEPROM.begin(baseAddress_ + capacityBytes_) ? eurorack::io::IoResult::Success
                                                           : eurorack::io::IoResult::DataError;
#else
        return eurorack::io::IoResult::Success;
#endif
    }

    /**
     * @brief Returns exposed capacity.
     *
     * @return Capacity in bytes.
     */
    [[nodiscard]] std::size_t capacity() const noexcept override {
        return capacityBytes_;
    }

    /**
     * @brief Reads bytes from EEPROM.
     *
     * @param address Relative starting address.
     * @param destination Destination buffer.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult read(const std::size_t address,
                                std::uint8_t* const destination,
                                const std::size_t size) const noexcept override {
        if ((destination == nullptr && size > 0U) || !validRange(address, size)) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        for (std::size_t index = 0U; index < size; ++index) {
            destination[index] = EEPROM.read(static_cast<int>(baseAddress_ + address + index));
        }

        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Writes bytes using EEPROM.update.
     *
     * @param address Relative starting address.
     * @param source Source buffer.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult write(const std::size_t address,
                                 const std::uint8_t* const source,
                                 const std::size_t size) noexcept override {
        if ((source == nullptr && size > 0U) || !validRange(address, size)) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        for (std::size_t index = 0U; index < size; ++index) {
            EEPROM.update(static_cast<int>(baseAddress_ + address + index), source[index]);
        }

        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Erases bytes using EEPROM.update.
     *
     * @param address Relative starting address.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult erase(const std::size_t address,
                                 const std::size_t size) noexcept override {
        if (!validRange(address, size)) {
            return eurorack::io::IoResult::InvalidArgument;
        }

        for (std::size_t index = 0U; index < size; ++index) {
            EEPROM.update(static_cast<int>(baseAddress_ + address + index), erasedValue_);
        }

        return eurorack::io::IoResult::Success;
    }

    /**
     * @brief Commits emulated EEPROM when required.
     *
     * @return Success when the requested operation completes; otherwise a specific validation, bus, or storage error.
     */
    eurorack::io::IoResult commit() noexcept override {
#if defined(ESP8266) || defined(ESP32)
        return EEPROM.commit() ? eurorack::io::IoResult::Success
                               : eurorack::io::IoResult::DataError;
#else
        return eurorack::io::IoResult::Success;
#endif
    }

    /**
     * @brief Returns the erase value.
     *
     * @return Erased byte value.
     */
    [[nodiscard]] std::uint8_t erasedValue() const noexcept override {
        return erasedValue_;
    }

  private:
    /**
     * @brief Returns remaining EEPROM capacity.
     *
     * @param baseAddress First exposed byte.
     * @return Remaining bytes.
     */
    [[nodiscard]] static std::size_t remainingCapacity(const std::size_t baseAddress) noexcept {
        const auto total = static_cast<std::size_t>(EEPROM.length());

        return baseAddress <= total ? total - baseAddress : 0U;
    }

    /**
     * @brief Bounds a requested capacity.
     *
     * @param baseAddress First exposed byte.
     * @param requested Requested bytes.
     * @return Bounded capacity.
     */
    [[nodiscard]] static std::size_t boundedCapacity(const std::size_t baseAddress,
                                                     const std::size_t requested) noexcept {
        const std::size_t remaining = remainingCapacity(baseAddress);

        return requested <= remaining ? requested : remaining;
    }

    /**
     * @brief Validates a relative address range.
     *
     * @param address Starting address.
     * @param size Number of bytes.
     * @return True when valid.
     */
    [[nodiscard]] bool validRange(const std::size_t address,
                                  const std::size_t size) const noexcept {
        return address <= capacityBytes_ && size <= capacityBytes_ - address;
    }

    std::size_t baseAddress_{0U};
    std::size_t capacityBytes_{0U};
    std::uint8_t erasedValue_{0xFFU};
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO
