/**
 * @file include/eurorack/storage/byte_codec.hpp
 * @brief Declares explicit little-endian byte encoding helpers.
 *
 * @details
 * Provides portable serialization primitives without dumping compiler-dependent C++ object layouts.
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

#include <cstddef>
#include <cstdint>

namespace eurorack::storage {

/**
 * @brief Writes primitive values into a bounded byte buffer using explicit little-endian layout.
 *
 * @details
 * Every write is bounds-checked before any byte is copied. Once any write fails, `good()` stays
 * false for the lifetime of the object; there is no way to clear the error.
 */
class ByteWriter final {
  public:
    /**
     * @brief Constructs a writer over caller-owned memory.
     *
     * @details
     * Starts in a failed state (`good()` returns false) if `destination` is null while
     * `capacity` is non-zero.
     *
     * @param destination Writable destination buffer; the writer does not own it.
     * @param capacity Destination buffer capacity in bytes.
     */
    ByteWriter(std::uint8_t* destination, std::size_t capacity) noexcept;

    /**
     * @brief Writes one byte.
     *
     * @param value Byte to write.
     * @return True if the byte fit within the remaining capacity; false if not, or if the
     * writer had already failed (see `good()`).
     */
    [[nodiscard]] bool writeU8(std::uint8_t value) noexcept;

    /**
     * @brief Writes one 16-bit value in little-endian byte order.
     *
     * @param value Value to write.
     * @return True if both bytes fit within the remaining capacity; false if not, or if the
     * writer had already failed (see `good()`).
     */
    [[nodiscard]] bool writeU16(std::uint16_t value) noexcept;

    /**
     * @brief Writes one 32-bit value in little-endian byte order.
     *
     * @param value Value to write.
     * @return True if all four bytes fit within the remaining capacity; false if not, or if the
     * writer had already failed (see `good()`).
     */
    [[nodiscard]] bool writeU32(std::uint32_t value) noexcept;

    /**
     * @brief Writes one signed 32-bit value using the same byte layout as `writeU32`.
     *
     * @param value Value to write, reinterpreted bitwise as `std::uint32_t`.
     * @return True if all four bytes fit within the remaining capacity; false if not, or if the
     * writer had already failed (see `good()`).
     */
    [[nodiscard]] bool writeI32(std::int32_t value) noexcept;

    /**
     * @brief Writes one IEEE-754 single-precision float using its raw bit pattern.
     *
     * @details
     * Copies the value's bit pattern via `memcpy` (no `reinterpret_cast`-based type punning)
     * and writes it with the same byte layout as `writeU32`.
     *
     * @param value Value to write.
     * @return True if all four bytes fit within the remaining capacity; false if not, or if the
     * writer had already failed (see `good()`).
     */
    [[nodiscard]] bool writeFloat(float value) noexcept;

    /**
     * @brief Writes a raw byte sequence.
     *
     * @details
     * All other `write*` methods are implemented in terms of this method. Fails without
     * modifying the destination buffer if `source` is null while `size` is non-zero, or if
     * `size` exceeds the remaining capacity; either failure permanently sets `good()` to false.
     *
     * @param source Bytes to copy; may be null only if `size` is zero.
     * @param size Number of bytes to write.
     * @return True if the bytes were copied; false otherwise.
     */
    [[nodiscard]] bool writeBytes(const std::uint8_t* source, std::size_t size) noexcept;

    /**
     * @brief Returns the number of bytes written so far.
     *
     * @return Current write position, in bytes from the start of the destination buffer.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Reports whether all previous write operations have succeeded.
     *
     * @return False if construction or any `write*` call has ever failed; true otherwise. Once
     * false, remains false permanently.
     */
    [[nodiscard]] bool good() const noexcept;

  private:
    std::uint8_t* destination_{nullptr}; ///< Caller-owned destination buffer; never allocated or
                                         ///< freed by this class.
    std::size_t capacity_{0U};           ///< Bytes available at `destination_`.
    std::size_t position_{0U};           ///< Bytes written so far; always at most `capacity_` while
                                         ///< `good_` is true.
    bool good_{true};                    ///< Sticky success flag; see `good()`.
};

/**
 * @brief Reads primitive values from a bounded byte buffer using explicit little-endian layout.
 *
 * @details
 * Every read is bounds-checked before any byte is copied. Once any read fails, `good()` stays
 * false for the lifetime of the object; there is no way to clear the error.
 */
class ByteReader final {
  public:
    /**
     * @brief Constructs a reader over caller-owned memory.
     *
     * @details
     * Starts in a failed state (`good()` returns false) if `source` is null while `size` is
     * non-zero.
     *
     * @param source Readable source buffer; the reader does not own it.
     * @param size Number of bytes available at `source`.
     */
    ByteReader(const std::uint8_t* source, std::size_t size) noexcept;

    /**
     * @brief Reads one byte.
     *
     * @param value Receives the byte on success; left unchanged on failure.
     * @return True if a byte was available; false if not, or if the reader had already failed
     * (see `good()`).
     */
    [[nodiscard]] bool readU8(std::uint8_t& value) noexcept;

    /**
     * @brief Reads one 16-bit value stored in little-endian byte order.
     *
     * @param value Receives the decoded value on success; left unchanged on failure.
     * @return True if both bytes were available; false if not, or if the reader had already
     * failed (see `good()`).
     */
    [[nodiscard]] bool readU16(std::uint16_t& value) noexcept;

    /**
     * @brief Reads one 32-bit value stored in little-endian byte order.
     *
     * @param value Receives the decoded value on success; left unchanged on failure.
     * @return True if all four bytes were available; false if not, or if the reader had already
     * failed (see `good()`).
     */
    [[nodiscard]] bool readU32(std::uint32_t& value) noexcept;

    /**
     * @brief Reads one signed 32-bit value using the same byte layout as `readU32`.
     *
     * @param value Receives the value, reinterpreted bitwise from the decoded
     * `std::uint32_t`; left unchanged on failure.
     * @return True if all four bytes were available; false if not, or if the reader had already
     * failed (see `good()`).
     */
    [[nodiscard]] bool readI32(std::int32_t& value) noexcept;

    /**
     * @brief Reads one IEEE-754 single-precision float from its raw bit pattern.
     *
     * @details
     * Decodes four bytes with the same layout as `readU32` and copies the resulting bit pattern
     * into `value` via `memcpy` (no `reinterpret_cast`-based type punning).
     *
     * @param value Receives the decoded value on success; left unchanged on failure.
     * @return True if all four bytes were available; false if not, or if the reader had already
     * failed (see `good()`).
     */
    [[nodiscard]] bool readFloat(float& value) noexcept;

    /**
     * @brief Reads a raw byte sequence.
     *
     * @details
     * All other `read*` methods are implemented in terms of this method. Fails without
     * modifying `destination` if it is null while `size` is non-zero, or if `size` exceeds the
     * remaining bytes; either failure permanently sets `good()` to false.
     *
     * @param destination Buffer to receive the bytes; may be null only if `size` is zero.
     * @param size Number of bytes to read.
     * @return True if the bytes were copied; false otherwise.
     */
    [[nodiscard]] bool readBytes(std::uint8_t* destination, std::size_t size) noexcept;

    /**
     * @brief Returns the number of unread bytes.
     *
     * @return Bytes remaining between the current read position and the end of the source
     * buffer.
     */
    [[nodiscard]] std::size_t remaining() const noexcept;

    /**
     * @brief Reports whether all previous read operations have succeeded.
     *
     * @return False if construction or any `read*` call has ever failed; true otherwise. Once
     * false, remains false permanently.
     */
    [[nodiscard]] bool good() const noexcept;

  private:
    const std::uint8_t* source_{nullptr}; ///< Caller-owned source buffer; never freed by this
                                          ///< class.
    std::size_t size_{0U};                ///< Total bytes available at `source_`.
    std::size_t position_{0U}; ///< Bytes read so far; always at most `size_` while `good_` is
                               ///< true.
    bool good_{true};          ///< Sticky success flag; see `good()`.
};

} // namespace eurorack::storage
