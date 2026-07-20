/**
 * @file src/storage/memory_storage.cpp
 * @brief Implements the in-memory persistent-storage backend.
 *
 * @details
 * Implements a deterministic EEPROM-like byte array for tests and embedded prototypes.
 * Writes are staged directly in the caller-independent in-memory buffer, erase restores the
 * configured erased value, and commit is a successful no-op because no second persistence layer
 * exists. Range checks are overflow-safe and zero-length operations are accepted when their start
 * address is valid.
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
#include <eurorack/storage/memory_storage.hpp>

namespace eurorack::storage {

MemoryStorage::MemoryStorage(const std::size_t capacityBytes, const std::uint8_t erasedValue)
    : bytes_(capacityBytes, erasedValue), erasedValue_(erasedValue) {}

std::size_t MemoryStorage::capacity() const noexcept {
    return bytes_.size();
}

eurorack::io::IoResult MemoryStorage::read(const std::size_t address,
                                           std::uint8_t* const destination,
                                           const std::size_t size) const noexcept {
    if (destination == nullptr && size > 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (!validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (size > 0U) {
        std::memcpy(destination, bytes_.data() + address, size);
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult MemoryStorage::write(const std::size_t address,
                                            const std::uint8_t* const source,
                                            const std::size_t size) noexcept {
    if (source == nullptr && size > 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (!validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (size > 0U) {
        std::memcpy(bytes_.data() + address, source, size);
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult MemoryStorage::erase(const std::size_t address,
                                            const std::size_t size) noexcept {
    if (!validRange(address, size)) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::fill(bytes_.begin() + static_cast<std::ptrdiff_t>(address),
              bytes_.begin() + static_cast<std::ptrdiff_t>(address + size),
              erasedValue_);

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult MemoryStorage::commit() noexcept {
    return eurorack::io::IoResult::Success;
}

std::uint8_t MemoryStorage::erasedValue() const noexcept {
    return erasedValue_;
}

const std::vector<std::uint8_t>& MemoryStorage::bytes() const noexcept {
    return bytes_;
}

bool MemoryStorage::validRange(const std::size_t address, const std::size_t size) const noexcept {
    return address <= bytes_.size() && size <= bytes_.size() - address;
}

} // namespace eurorack::storage
