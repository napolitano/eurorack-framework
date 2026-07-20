/**
 * @file src/storage/file_storage.cpp
 * @brief Implements file-backed persistent storage.
 *
 * @details
 * Stages all bytes in memory and commits through a temporary file followed by filesystem
 * replacement.
 *
 * The in-memory image has a fixed capacity and mirrors the semantics of an
 * EEPROM-like byte device. Writes modify only the staged image and set the
 * dirty flag. commit() writes a sibling temporary file, flushes and closes it,
 * then replaces the target path. The operation is designed for deterministic
 * native tests and simulators, not for real-time firmware.
 *
 * `std::filesystem`, exceptions, heap allocation, and blocking file I/O make
 * this backend intentionally native-only.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstring.hpp>
#else
#include <cstring>
#endif
#include <eurorack/storage/file_storage.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace eurorack::storage {

FileStorage::FileStorage(std::string path,
                         const std::size_t capacityBytes,
                         const std::uint8_t erasedValue) noexcept
    : path_(std::move(path)), bytes_(capacityBytes, erasedValue), erasedValue_(erasedValue) {
    ready_ = loadExisting();
}

bool FileStorage::ready() const noexcept {
    return ready_;
}

std::size_t FileStorage::capacity() const noexcept {
    return bytes_.size();
}

eurorack::io::IoResult FileStorage::read(const std::size_t address,
                                         std::uint8_t* const destination,
                                         const std::size_t size) const noexcept {
    if (!ready_) {
        return eurorack::io::IoResult::DataError;
    }

    if ((destination == nullptr && size > 0U) || !validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (size > 0U) {
        std::memcpy(destination, bytes_.data() + address, size);
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult FileStorage::write(const std::size_t address,
                                          const std::uint8_t* const source,
                                          const std::size_t size) noexcept {
    if (!ready_) {
        return eurorack::io::IoResult::DataError;
    }

    if ((source == nullptr && size > 0U) || !validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (size > 0U && std::memcmp(bytes_.data() + address, source, size) != 0) {
        std::memcpy(bytes_.data() + address, source, size);
        dirty_ = true;
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult FileStorage::erase(const std::size_t address,
                                          const std::size_t size) noexcept {
    if (!ready_) {
        return eurorack::io::IoResult::DataError;
    }

    if (!validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    const auto begin = bytes_.begin() + static_cast<std::ptrdiff_t>(address);
    const auto end = begin + static_cast<std::ptrdiff_t>(size);

    const bool alreadyErased =
        std::all_of(begin, end, [this](const std::uint8_t value) { return value == erasedValue_; });

    if (!alreadyErased) {
        std::fill(begin, end, erasedValue_);
        dirty_ = true;
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult FileStorage::commit() noexcept {
    if (!ready_) {
        return eurorack::io::IoResult::DataError;
    }

    if (!dirty_) {
        return eurorack::io::IoResult::Success;
    }

    try {
        // Use a sibling temporary file so that the final rename normally
        // remains within one filesystem. Cross-filesystem rename is not atomic.
        const std::filesystem::path target(path_);
        const std::filesystem::path temporary = target.string() + ".tmp";

        if (target.has_parent_path()) {
            std::error_code directoryError;
            std::filesystem::create_directories(target.parent_path(), directoryError);

            if (directoryError) {
                return eurorack::io::IoResult::DataError;
            }
        }

        {
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);

            if (!output) {
                return eurorack::io::IoResult::DataError;
            }

            output.write(reinterpret_cast<const char*>(bytes_.data()),
                         static_cast<std::streamsize>(bytes_.size()));
            output.flush();

            if (!output) {
                return eurorack::io::IoResult::DataError;
            }
        }

        // Windows does not consistently replace an existing file with rename().
        // Remove first, then rename. This creates a short interval without the
        // target file and is therefore weaker than POSIX rename replacement.
        std::error_code removeError;
        std::filesystem::remove(target, removeError);

        std::error_code renameError;
        std::filesystem::rename(temporary, target, renameError);

        if (renameError) {
            std::error_code cleanupError;
            std::filesystem::remove(temporary, cleanupError);
            return eurorack::io::IoResult::DataError;
        }

        dirty_ = false;
        return eurorack::io::IoResult::Success;
    } catch (...) {
        return eurorack::io::IoResult::UnknownError;
    }
}

std::uint8_t FileStorage::erasedValue() const noexcept {
    return erasedValue_;
}

bool FileStorage::dirty() const noexcept {
    return dirty_;
}

const std::string& FileStorage::path() const noexcept {
    return path_;
}

bool FileStorage::validRange(const std::size_t address, const std::size_t size) const noexcept {
    return address <= bytes_.size() && size <= bytes_.size() - address;
}

bool FileStorage::loadExisting() noexcept {
    try {
        const std::filesystem::path target(path_);

        if (!std::filesystem::exists(target)) {
            dirty_ = false;
            return true;
        }

        std::ifstream input(target, std::ios::binary);
        if (!input) {
            return false;
        }

        input.read(reinterpret_cast<char*>(bytes_.data()),
                   static_cast<std::streamsize>(bytes_.size()));

        const std::streamsize loaded = input.gcount();
        if (loaded < 0) {
            return false;
        }

        const auto loadedSize = static_cast<std::size_t>(loaded);

        if (loadedSize < bytes_.size()) {
            std::fill(bytes_.begin() + static_cast<std::ptrdiff_t>(loadedSize),
                      bytes_.end(),
                      erasedValue_);
        }

        dirty_ = false;
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace eurorack::storage

#endif // !defined(ARDUINO)
