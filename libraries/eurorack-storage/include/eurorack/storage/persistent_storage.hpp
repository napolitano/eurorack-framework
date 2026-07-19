/**
 * @file include/eurorack/storage/persistent_storage.hpp
 * @brief Declares a platform-independent persistent byte-storage interface.
 *
 * @details
 * Provides bounded reads, writes, erase, and commit semantics for EEPROM, flash, files, and
 * simulators.
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
 * @ingroup storage
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/io_result.hpp>

namespace eurorack::storage {

/**
 * @brief Abstract persistent byte-storage backend.
 */
class PersistentStorage {
  public:
    /**
     * @brief Destroys the storage backend.
     */
    virtual ~PersistentStorage() = default;

    /**
     * @brief Returns total addressable storage capacity.
     *
     * @return Capacity in bytes.
     */
    [[nodiscard]] virtual std::size_t capacity() const noexcept = 0;

    /**
     * @brief Reads bytes from persistent storage.
     *
     * @param address Starting byte address.
     * @param destination Destination buffer.
     * @param size Number of bytes to read.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    virtual eurorack::io::IoResult
    read(std::size_t address, std::uint8_t* destination, std::size_t size) const noexcept = 0;

    /**
     * @brief Writes bytes to persistent storage.
     *
     * @param address Starting byte address.
     * @param source Source buffer.
     * @param size Number of bytes to write.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    virtual eurorack::io::IoResult
    write(std::size_t address, const std::uint8_t* source, std::size_t size) noexcept = 0;

    /**
     * @brief Restores a byte range to the backend erase value.
     *
     * @param address Starting byte address.
     * @param size Number of bytes to erase.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    virtual eurorack::io::IoResult erase(std::size_t address, std::size_t size) noexcept = 0;

    /**
     * @brief Commits pending writes when required by the backend.
     *
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    virtual eurorack::io::IoResult commit() noexcept = 0;

    /**
     * @brief Returns the value produced by an erased byte.
     *
     * @return Erased byte value, commonly 0xFF.
     */
    [[nodiscard]] virtual std::uint8_t erasedValue() const noexcept = 0;
};

} // namespace eurorack::storage
