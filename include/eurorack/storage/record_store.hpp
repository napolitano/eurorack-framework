/**
 * @file include/eurorack/storage/record_store.hpp
 * @brief Declares versioned, CRC-protected, dual-slot persistent records.
 *
 * @details
 * Provides explicit byte payload persistence with sequence selection and atomic slot replacement.
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
#include <eurorack/io/io_result.hpp>
#include <eurorack/storage/persistent_storage.hpp>

namespace eurorack::storage {

/**
 * @brief Result of loading or storing a record.
 */
enum class RecordStatus : std::uint8_t {
    Success,
    NoValidRecord,
    InvalidArgument,
    StorageError,
    PayloadTooLarge,
    VersionMismatch,
    CorruptRecord
};

/**
 * @brief Fixed record metadata written before each payload.
 */
struct RecordHeader final {
    std::uint32_t magic{0U};         ///< Must equal `RecordStoreConfig::magic` for the slot to be
                                     ///< considered valid.
    std::uint16_t schemaVersion{0U}; ///< Payload schema version as stored; compared against
                                     ///< `RecordStoreConfig::schemaVersion` by `load`, but not
                                     ///< checked by `validateHeader`.
    std::uint16_t payloadSize{0U};   ///< Payload size in bytes; must not exceed
                                     ///< `RecordStore::maximumPayloadSize()` for the slot to be
                                     ///< considered valid.
    std::uint32_t sequence{0U};     ///< Monotonically increasing save counter, compared with signed
                                    ///< wraparound-safe arithmetic to select the newer of two valid
                                    ///< slots.
    std::uint32_t payloadCrc32{0U}; ///< CRC-32 of the payload bytes alone.
    std::uint32_t headerCrc32{0U};  ///< CRC-32 of this header with `headerCrc32` itself treated
                                    ///< as zero during calculation.
};

/**
 * @brief Configuration for a dual-slot record region.
 */
struct RecordStoreConfig final {
    std::size_t baseAddress{0U}; ///< Storage address of the first slot; the second slot
                                 ///< immediately follows at `baseAddress + slotSize`.
    std::size_t slotSize{256U};  ///< Bytes reserved per slot, including the `RecordHeader`; the
                                 ///< maximum payload is `slotSize - sizeof(RecordHeader)`.
    std::uint32_t magic{0x4555524FU}; ///< Value every valid `RecordHeader::magic` must match.
    std::uint16_t schemaVersion{1U};  ///< Expected payload schema version; `load` reports
                                      ///< `VersionMismatch` when a selected record's stored
                                      ///< version differs.
};

/**
 * @brief Load result including metadata diagnostics.
 */
struct RecordLoadResult final {
    RecordStatus status{RecordStatus::NoValidRecord}; ///< Outcome of the `load` call; only
                                                      ///< `Success` guarantees `destination`
                                                      ///< was written.
    std::size_t payloadSize{0U}; ///< Payload size of the selected record, if any was found;
                                 ///< valid even when `status` is `VersionMismatch` or
                                 ///< `PayloadTooLarge`.
    std::uint32_t sequence{0U};  ///< Sequence number of the selected record, if any was found.
    std::uint16_t storedSchemaVersion{0U}; ///< Schema version stored in the selected record, if
                                           ///< any was found; compare against the store's
                                           ///< configured `schemaVersion` to explain a
                                           ///< `VersionMismatch` status.
};

/**
 * @brief Stores explicit byte payloads redundantly in two CRC-protected, sequence-numbered slots.
 *
 * @details
 * Each of the two fixed-size slots holds a `RecordHeader` followed by a payload. `store` always
 * writes the older or currently-invalid slot and leaves the other slot untouched until the new
 * one is fully written and the backend has committed, so a power loss mid-write cannot corrupt
 * the last good record. `load` independently validates both slots and selects the newer valid
 * one by sequence number (compared with wraparound-safe signed arithmetic), so a corrupt slot
 * never prevents the redundant copy from being used.
 */
class RecordStore final {
  public:
    /**
     * @brief Constructs a record store over an existing backend.
     *
     * @param storage Persistent byte-storage backend covering at least
     * `2 * config.slotSize` bytes from `config.baseAddress`; the store does not own it.
     * @param config Slot layout and record identity.
     */
    RecordStore(PersistentStorage& storage, RecordStoreConfig config) noexcept;

    /**
     * @brief Returns maximum payload size per slot.
     *
     * @return `slotSize - sizeof(RecordHeader)` from the configuration, or `0` if `slotSize` is
     * not large enough to hold a header.
     */
    [[nodiscard]] std::size_t maximumPayloadSize() const noexcept;

    /**
     * @brief Loads the newest valid record's payload.
     *
     * @details
     * Validates both slots independently (magic, size bound, header CRC, and payload CRC) and
     * selects the one with the newer sequence number if both are valid. Returns
     * `NoValidRecord` if neither slot validates, `VersionMismatch` if the selected record's
     * schema version differs from the configured one, or `PayloadTooLarge` if
     * `destinationCapacity` is smaller than the record's payload. `payloadSize`, `sequence`, and
     * `storedSchemaVersion` in the result are populated whenever a valid record was selected,
     * even for `VersionMismatch` and `PayloadTooLarge`.
     *
     * @param destination Destination payload buffer; may be null only if `destinationCapacity`
     * is zero. Only written when the result status is `Success`.
     * @param destinationCapacity Destination buffer capacity in bytes.
     * @return Load result and record metadata.
     */
    [[nodiscard]] RecordLoadResult load(std::uint8_t* destination,
                                        std::size_t destinationCapacity) const noexcept;

    /**
     * @brief Stores one payload atomically, writing whichever slot is not the current record.
     *
     * @details
     * If exactly one slot currently holds a valid record, the payload is written to the other
     * slot with `sequence` one greater than the existing record's. If both slots are valid, the
     * older one is overwritten. If neither is valid, slot 0 is used with `sequence == 1`. The
     * target slot is erased, the payload and header are written, and the backend is committed,
     * in that order; the previously valid slot is left untouched throughout, so a failure or
     * power loss during this call cannot destroy the last good record.
     *
     * @param payload Source payload; may be null only if `payloadSize` is zero.
     * @param payloadSize Number of payload bytes; rejected with `PayloadTooLarge` if it exceeds
     * `maximumPayloadSize()` or 0xFFFF.
     * @return `Success` once the new record is written and committed; `InvalidArgument` for a
     * null payload with non-zero size; `PayloadTooLarge` if the size is rejected; otherwise
     * `StorageError` if any backend erase, write, or commit fails.
     */
    [[nodiscard]] RecordStatus store(const std::uint8_t* payload, std::size_t payloadSize) noexcept;

    /**
     * @brief Erases both record slots and commits the change.
     *
     * @details
     * After a successful call, `load` reports `NoValidRecord` until the next successful `store`.
     *
     * @return `Success` once both slots are erased and the backend has committed; otherwise
     * `StorageError`.
     */
    [[nodiscard]] RecordStatus clear() noexcept;

  private:
    /**
     * @brief Internal validation result for one redundant storage slot.
     *
     * @details
     * The structure combines decoded header metadata, validity, and physical
     * slot index. Payload bytes are not retained here; they are read only after
     * the selected slot has passed every validation check.
     */
    struct SlotInfo final {
        bool valid{false};       ///< True when the slot passed magic, size, header-CRC, and
                                 ///< payload-CRC validation in `inspectSlot`.
        std::size_t address{0U}; ///< Storage address of this slot, as returned by
                                 ///< `slotAddress`.
        RecordHeader header{};   ///< Header as read from storage; only meaningful together with
                                 ///< `valid`.
    };

    /**
     * @brief Reads and fully validates one slot, including its payload CRC.
     *
     * @details
     * Returns a `SlotInfo` with `valid` false if the header cannot be read, fails
     * `validateHeader`, or if the payload cannot be read or its CRC-32 does not match
     * `RecordHeader::payloadCrc32`. Reading the full payload here (rather than only the header)
     * is what allows `store` and `load` to trust `sequence` only for slots proven intact.
     *
     * @param slotIndex Zero-based record slot index (0 or 1).
     * @return Validation result and decoded header for the slot.
     */
    [[nodiscard]] SlotInfo inspectSlot(std::size_t slotIndex) const noexcept;

    /**
     * @brief Reads the fixed-size header from a slot address.
     *
     * @param address Storage address of the slot.
     * @param header Receives the decoded header on success; contents are undefined on failure.
     * @return True if the underlying storage read succeeded.
     */
    [[nodiscard]] bool readHeader(std::size_t address, RecordHeader& header) const noexcept;

    /**
     * @brief Validates structural header fields without touching the payload.
     *
     * @details
     * Checks that `magic` matches the configured value, that `payloadSize` does not exceed
     * `maximumPayloadSize()`, and that `headerCrc32` matches `calculateHeaderCrc`.
     *
     * @param header Header to validate.
     * @return True when all structural checks pass.
     */
    [[nodiscard]] bool validateHeader(const RecordHeader& header) const noexcept;

    /**
     * @brief Calculates the CRC-32 used to detect header corruption.
     *
     * @details
     * Computes the CRC-32 of `header` with its own `headerCrc32` field treated as zero, so the
     * same function both produces and later verifies the stored value.
     *
     * @param header Header to checksum.
     * @return CRC-32 of the header with `headerCrc32` zeroed.
     */
    [[nodiscard]] std::uint32_t calculateHeaderCrc(const RecordHeader& header) const noexcept;

    /**
     * @brief Calculates the storage address of a slot.
     *
     * @param slotIndex Zero-based record slot index (0 or 1).
     * @return `config_.baseAddress + slotIndex * config_.slotSize`.
     */
    [[nodiscard]] std::size_t slotAddress(std::size_t slotIndex) const noexcept;

    PersistentStorage& storage_; ///< Backend used for every slot read, write, erase, and commit;
                                 ///< the store does not own it.
    RecordStoreConfig config_{}; ///< Slot layout, magic, and expected schema version.
};

} // namespace eurorack::storage
