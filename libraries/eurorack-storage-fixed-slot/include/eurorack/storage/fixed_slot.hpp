/**
 * @file include/eurorack/storage/fixed_slot.hpp
 * @brief Declares a heap-free, single-slot persistent value backed by a validity marker.
 *
 * @details
 * Stores one fixed-size payload at a fixed storage address behind a single validity marker byte,
 * with no dynamic allocation anywhere in its implementation. The marker is written invalid before
 * the payload and only written valid again after the payload has been committed, so a power loss
 * or reset during `store()` leaves the slot correctly reporting as invalid rather than silently
 * exposing a torn write. This class provides no redundancy or checksum; `eurorack::storage::
 * RecordStore` offers a dual-slot, CRC-protected alternative for callers who need that and can
 * accept its `std::vector`-based implementation. The storage backend is a non-owning dependency
 * and must outlive this object. Methods are synchronous and are not internally synchronized.
 *
 * @ingroup storage
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstddef>
#include <cstdint>
#endif
#include <eurorack/storage/persistent_storage.hpp>

namespace eurorack::storage {

/**
 * @brief Address, payload size, and validity marker for one `FixedSlot`.
 */
struct FixedSlotConfig final {
    std::size_t address{0U};     ///< Storage address of the marker byte; the payload immediately
                                 ///< follows at `address + 1`.
    std::size_t payloadSize{0U}; ///< Exact payload size in bytes that `store()` and `load()`
                                 ///< require.
    std::uint8_t marker{0xAAU};  ///< Byte value that marks the slot as holding a valid payload.
};

/**
 * @brief Heap-free, single fixed-size persistent slot with a marker-last write order.
 */
class FixedSlot final {
  public:
    /**
     * @brief Constructs a fixed slot over an existing storage backend.
     *
     * @param storage Persistent byte-storage backend covering at least
     * `config.payloadSize + 1` bytes from `config.address`; this object does not own it.
     * @param config Address, payload size, and validity marker.
     */
    FixedSlot(PersistentStorage& storage, FixedSlotConfig config) noexcept
        : storage_(storage), config_(config) {}

    /**
     * @brief Stores one payload, marking the slot invalid until the write is complete.
     *
     * @details
     * Writes an invalid marker byte first, then the payload (if non-empty), then the configured
     * valid marker byte, and finally commits the backend. If a failure or power loss interrupts
     * any step before the valid marker is written, the slot is left holding the invalid marker
     * and `valid()`/`load()` correctly report it as such.
     *
     * @param payload Source payload; may be null only if `size` is zero.
     * @param size Number of payload bytes; must exactly equal `config.payloadSize`.
     * @return `InvalidArgument` for a null payload with non-zero size or a `size` that does not
     * match `config.payloadSize`; otherwise `Success` once the marker, payload, and commit all
     * succeed, or the first backend error encountered.
     */
    eurorack::io::IoResult store(const std::uint8_t* payload, std::size_t size) noexcept;

    /**
     * @brief Loads the payload if the slot currently holds a valid marker.
     *
     * @param payload Destination buffer; may be null only if `capacity` is zero. Only written
     * when the slot is valid and the read succeeds.
     * @param capacity Destination buffer capacity in bytes; must be at least
     * `config.payloadSize`.
     * @return `InvalidArgument` for a null destination with non-zero capacity or a `capacity`
     * smaller than `config.payloadSize`; `Busy` if the stored marker does not match
     * `config.marker`; otherwise `Success` once the payload is read, or the backend error
     * encountered.
     */
    eurorack::io::IoResult load(std::uint8_t* payload, std::size_t capacity) const noexcept;

    /**
     * @brief Invalidates the slot without touching the payload bytes.
     *
     * @details
     * Writes the invalid marker byte and commits. The payload bytes themselves are left as-is;
     * only the marker changes, which is sufficient to make `valid()`/`load()` treat the slot as
     * empty.
     *
     * @return `Success` once the marker write and commit succeed; otherwise the backend error
     * encountered.
     */
    eurorack::io::IoResult clear() noexcept;

    /**
     * @brief Checks whether the slot currently holds the configured valid marker.
     *
     * @return True if the stored marker byte was read successfully and equals `config.marker`;
     * false on a read failure or a non-matching marker.
     */
    [[nodiscard]] bool valid() const noexcept;

  private:
    PersistentStorage& storage_; ///< Backend used for every marker, payload, and commit
                                 ///< operation; this object does not own it.
    FixedSlotConfig config_{};   ///< Address, payload size, and validity marker.
};

} // namespace eurorack::storage
