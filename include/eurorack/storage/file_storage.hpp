/**
 * @file include/eurorack/storage/file_storage.hpp
 * @brief Declares a file-backed persistent-storage backend.
 *
 * @details
 * Provides bounded in-memory staging with atomic replacement on commit for native tests,
 * simulators, and desktop tools.
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

#if !defined(ARDUINO)

#pragma once

#include <cstddef>
#include <cstdint>
#include <eurorack/storage/persistent_storage.hpp>
#include <string>
#include <vector>

namespace eurorack::storage {

/**
 * @brief File-backed persistent byte storage.
 */
class FileStorage final : public PersistentStorage {
  public:
    /**
     * @brief Constructs a file-backed storage region.
     *
     * @param path Target file path.
     * @param capacityBytes Fixed addressable capacity.
     * @param erasedValue Value used for new and erased bytes.
     */
    FileStorage(std::string path,
                std::size_t capacityBytes,
                std::uint8_t erasedValue = 0xFFU) noexcept;

    /**
     * @brief Returns whether construction produced a usable image.
     *
     * @return True when ready.
     */
    [[nodiscard]] bool ready() const noexcept;

    /**
     * @brief Returns total addressable capacity.
     *
     * @return Capacity in bytes.
     */
    [[nodiscard]] std::size_t capacity() const noexcept override;

    /**
     * @brief Reads bytes from the staged image.
     *
     * @param address Starting address.
     * @param destination Destination buffer.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult
    read(std::size_t address, std::uint8_t* destination, std::size_t size) const noexcept override;

    /**
     * @brief Writes bytes into the staged image.
     *
     * @param address Starting address.
     * @param source Source buffer.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult
    write(std::size_t address, const std::uint8_t* source, std::size_t size) noexcept override;

    /**
     * @brief Restores bytes to the erased value.
     *
     * @param address Starting address.
     * @param size Number of bytes.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult erase(std::size_t address, std::size_t size) noexcept override;

    /**
     * @brief Atomically writes the staged image to disk.
     *
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    eurorack::io::IoResult commit() noexcept override;

    /**
     * @brief Returns the erased byte value.
     *
     * @return Erased byte value.
     */
    [[nodiscard]] std::uint8_t erasedValue() const noexcept override;

    /**
     * @brief Returns whether uncommitted changes exist.
     *
     * @return True when dirty.
     */
    [[nodiscard]] bool dirty() const noexcept;

    /**
     * @brief Returns the configured target path.
     *
     * @return Constant reference to the path.
     */
    [[nodiscard]] const std::string& path() const noexcept;

  private:
    /**
     * @brief Validates one address range.
     *
     * @param address Starting address.
     * @param size Number of bytes.
     * @return True when valid.
     */
    [[nodiscard]] bool validRange(std::size_t address, std::size_t size) const noexcept;

    /**
     * @brief Loads an existing file image.
     *
     * @return True on success.
     */
    [[nodiscard]] bool loadExisting() noexcept;

    std::string path_{};
    std::vector<std::uint8_t> bytes_{};
    std::uint8_t erasedValue_{0xFFU};
    bool ready_{false};
    bool dirty_{false};
};

} // namespace eurorack::storage

#endif // !defined(ARDUINO)
