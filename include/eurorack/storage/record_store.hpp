/**
 * @file include/eurorack/storage/record_store.hpp
 * @brief Declares versioned, CRC-protected, dual-slot persistent records.
 *
 * @details
 * Provides explicit byte payload persistence with sequence selection and atomic slot replacement.
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
    std::uint32_t magic{0U};
    std::uint16_t schemaVersion{0U};
    std::uint16_t payloadSize{0U};
    std::uint32_t sequence{0U};
    std::uint32_t payloadCrc32{0U};
    std::uint32_t headerCrc32{0U};
};

/**
 * @brief Configuration for a dual-slot record region.
 */
struct RecordStoreConfig final {
    std::size_t baseAddress{0U};
    std::size_t slotSize{256U};
    std::uint32_t magic{0x4555524FU};
    std::uint16_t schemaVersion{1U};
};

/**
 * @brief Load result including metadata diagnostics.
 */
struct RecordLoadResult final {
    RecordStatus status{RecordStatus::NoValidRecord};
    std::size_t payloadSize{0U};
    std::uint32_t sequence{0U};
    std::uint16_t storedSchemaVersion{0U};
};

/**
 * @brief Stores explicit byte payloads in two CRC-protected slots.
 */
class RecordStore final {
  public:
    /**
     * @brief Constructs a record store over an existing backend.
     *
     * @param storage Persistent byte-storage backend.
     * @param config Slot layout and record identity.
     */
    RecordStore(PersistentStorage& storage, RecordStoreConfig config) noexcept;

    /**
     * @brief Returns maximum payload size per slot.
     *
     * @return Maximum payload bytes.
     */
    [[nodiscard]] std::size_t maximumPayloadSize() const noexcept;

    /**
     * @brief Loads the newest valid record.
     *
     * @param destination Destination payload buffer.
     * @param destinationCapacity Destination buffer capacity.
     * @return Load result and record metadata.
     */
    [[nodiscard]] RecordLoadResult load(std::uint8_t* destination,
                                        std::size_t destinationCapacity) const noexcept;

    /**
     * @brief Stores one explicit byte payload atomically across two slots.
     *
     * @param payload Source payload.
     * @param payloadSize Number of payload bytes.
     * @return Record status.
     */
    [[nodiscard]] RecordStatus store(const std::uint8_t* payload, std::size_t payloadSize) noexcept;

    /**
     * @brief Erases both record slots.
     *
     * @return Record status.
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
        bool valid{false};
        std::size_t address{0U};
        RecordHeader header{};
    };

    /**
     * @brief Inspects slot.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param slotIndex Zero-based record slot index.
     *
     * @return The value described by the operation.
     */
    [[nodiscard]] SlotInfo inspectSlot(std::size_t slotIndex) const noexcept;

    /**
     * @brief Reads header.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param address Storage address, register address, or I2C address as defined by the enclosing API.
     *
     * @param header Record header read, validated, or checksummed.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool readHeader(std::size_t address, RecordHeader& header) const noexcept;

    /**
     * @brief Validates header.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param header Record header read, validated, or checksummed.
     *
     * @return True when the documented condition is satisfied; otherwise false.
     */
    [[nodiscard]] bool validateHeader(const RecordHeader& header) const noexcept;

    /**
     * @brief Calculates header crc.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param header Record header read, validated, or checksummed.
     *
     * @return The requested integer value in the units documented by the enclosing API.
     */
    [[nodiscard]] std::uint32_t calculateHeaderCrc(const RecordHeader& header) const noexcept;

    /**
     * @brief Provides the slot address operation.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param slotIndex Zero-based record slot index.
     *
     * @return The requested count or index in bytes or elements.
     */
    [[nodiscard]] std::size_t slotAddress(std::size_t slotIndex) const noexcept;

    PersistentStorage& storage_;
    RecordStoreConfig config_{};
};

} // namespace eurorack::storage
