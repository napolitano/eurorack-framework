/**
 * @file src/storage/byte_codec.cpp
 * @brief Implements explicit little-endian byte encoding helpers.
 *
 * @details
 * Serializes fixed-width integers in a defined byte order and reconstructs them without relying on
 * host alignment or endianness. The helpers advance caller-provided offsets only after validating
 * the complete requested range. This makes them suitable for persistent record layouts that must
 * remain stable across desktop tests and microcontroller targets. No reinterpret_cast-based type
 * punning is used.
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
#include <eurorack/compat/avr/cstring.hpp>
#else
#include <cstring>
#endif
#include <eurorack/storage/byte_codec.hpp>

namespace eurorack::storage {

ByteWriter::ByteWriter(std::uint8_t* const destination, const std::size_t capacity) noexcept
    : destination_(destination), capacity_(capacity),
      good_(destination != nullptr || capacity == 0U) {}

bool ByteWriter::writeU8(const std::uint8_t value) noexcept {
    return writeBytes(&value, 1U);
}

bool ByteWriter::writeU16(const std::uint16_t value) noexcept {
    const std::uint8_t bytes[2] = {
        static_cast<std::uint8_t>(value & 0xFFU),
        static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
    };

    return writeBytes(bytes, sizeof(bytes));
}

bool ByteWriter::writeU32(const std::uint32_t value) noexcept {
    const std::uint8_t bytes[4] = {
        static_cast<std::uint8_t>(value & 0xFFU),
        static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
        static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
        static_cast<std::uint8_t>((value >> 24U) & 0xFFU),
    };

    return writeBytes(bytes, sizeof(bytes));
}

bool ByteWriter::writeI32(const std::int32_t value) noexcept {
    return writeU32(static_cast<std::uint32_t>(value));
}

bool ByteWriter::writeFloat(const float value) noexcept {
    static_assert(sizeof(float) == sizeof(std::uint32_t));

    std::uint32_t bits = 0U;
    std::memcpy(&bits, &value, sizeof(bits));

    return writeU32(bits);
}

bool ByteWriter::writeBytes(const std::uint8_t* const source, const std::size_t size) noexcept {
    if (!good_ || (source == nullptr && size > 0U) || size > capacity_ - position_) {
        good_ = false;
        return false;
    }

    if (size > 0U) {
        std::memcpy(destination_ + position_, source, size);
        position_ += size;
    }

    return true;
}

std::size_t ByteWriter::size() const noexcept {
    return position_;
}

bool ByteWriter::good() const noexcept {
    return good_;
}

ByteReader::ByteReader(const std::uint8_t* const source, const std::size_t size) noexcept
    : source_(source), size_(size), good_(source != nullptr || size == 0U) {}

bool ByteReader::readU8(std::uint8_t& value) noexcept {
    return readBytes(&value, 1U);
}

bool ByteReader::readU16(std::uint16_t& value) noexcept {
    std::uint8_t bytes[2]{};

    if (!readBytes(bytes, sizeof(bytes))) {
        return false;
    }

    value = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[0]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[1]) << 8U));

    return true;
}

bool ByteReader::readU32(std::uint32_t& value) noexcept {
    std::uint8_t bytes[4]{};

    if (!readBytes(bytes, sizeof(bytes))) {
        return false;
    }

    value = static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8U) |
            (static_cast<std::uint32_t>(bytes[2]) << 16U) |
            (static_cast<std::uint32_t>(bytes[3]) << 24U);

    return true;
}

bool ByteReader::readI32(std::int32_t& value) noexcept {
    std::uint32_t bits = 0U;

    if (!readU32(bits)) {
        return false;
    }

    value = static_cast<std::int32_t>(bits);
    return true;
}

bool ByteReader::readFloat(float& value) noexcept {
    static_assert(sizeof(float) == sizeof(std::uint32_t));

    std::uint32_t bits = 0U;

    if (!readU32(bits)) {
        return false;
    }

    std::memcpy(&value, &bits, sizeof(value));
    return true;
}

bool ByteReader::readBytes(std::uint8_t* const destination, const std::size_t size) noexcept {
    if (!good_ || (destination == nullptr && size > 0U) || size > size_ - position_) {
        good_ = false;
        return false;
    }

    if (size > 0U) {
        std::memcpy(destination, source_ + position_, size);
        position_ += size;
    }

    return true;
}

std::size_t ByteReader::remaining() const noexcept {
    return size_ - position_;
}

bool ByteReader::good() const noexcept {
    return good_;
}

} // namespace eurorack::storage
