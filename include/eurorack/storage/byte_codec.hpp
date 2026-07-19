/**
 * @file include/eurorack/storage/byte_codec.hpp
 * @brief Declares explicit little-endian byte encoding helpers.
 *
 * @details
 * Provides portable serialization primitives without dumping compiler-dependent C++ object layouts.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup storage
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace eurorack::storage {

/**
 * @brief Writes primitive values into a bounded byte buffer.
 */
class ByteWriter final {
  public:
    /**
     * @brief Provides the byte writer operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param destination Writable destination buffer.
     *
     * @param capacity Destination buffer capacity in bytes.
     */
    ByteWriter(std::uint8_t* destination, std::size_t capacity) noexcept;

    /**
     * @brief Writes u8.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeU8(std::uint8_t value) noexcept;
    /**
     * @brief Writes u16.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeU16(std::uint16_t value) noexcept;
    /**
     * @brief Writes u32.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeU32(std::uint32_t value) noexcept;
    /**
     * @brief Writes i32.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeI32(std::int32_t value) noexcept;
    /**
     * @brief Writes float.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeFloat(float value) noexcept;
    /**
     * @brief Writes bytes.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param source Readable source buffer.
     *
     * @param size Number of bytes or elements involved.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool writeBytes(const std::uint8_t* source, std::size_t size) noexcept;

    /**
     * @brief Provides the size operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested count or index in bytes or elements.
     */
    [[nodiscard]] std::size_t size() const noexcept;
    /**
     * @brief Reports whether all previous codec operations succeeded.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool good() const noexcept;

  private:
    std::uint8_t* destination_{nullptr};
    std::size_t capacity_{0U};
    std::size_t position_{0U};
    bool good_{true};
};

/**
 * @brief Reads primitive values from a bounded byte buffer.
 */
class ByteReader final {
  public:
    /**
     * @brief Provides the byte reader operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param source Readable source buffer.
     *
     * @param size Number of bytes or elements involved.
     */
    ByteReader(const std::uint8_t* source, std::size_t size) noexcept;

    /**
     * @brief Reads u8.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readU8(std::uint8_t& value) noexcept;
    /**
     * @brief Reads u16.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readU16(std::uint16_t& value) noexcept;
    /**
     * @brief Reads u32.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readU32(std::uint32_t& value) noexcept;
    /**
     * @brief Reads i32.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readI32(std::int32_t& value) noexcept;
    /**
     * @brief Reads float.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param value Value read, written, or updated by the operation.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readFloat(float& value) noexcept;
    /**
     * @brief Reads bytes.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param destination Writable destination buffer.
     *
     * @param size Number of bytes or elements involved.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readBytes(std::uint8_t* destination, std::size_t size) noexcept;

    /**
     * @brief Returns the unread byte count.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return The requested count or index in bytes or elements.
     */
    [[nodiscard]] std::size_t remaining() const noexcept;
    /**
     * @brief Reports whether all previous codec operations succeeded.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool good() const noexcept;

  private:
    const std::uint8_t* source_{nullptr};
    std::size_t size_{0U};
    std::size_t position_{0U};
    bool good_{true};
};

} // namespace eurorack::storage
