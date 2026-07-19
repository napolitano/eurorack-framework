/**
 * @file src/storage/record_store.cpp
 * @brief Implements versioned, CRC-protected, dual-slot records.
 *
 * @details
 * Selects the newest valid sequence and writes replacements to the inactive slot.
 *
 * The store uses two fixed-size slots. Each slot contains a header followed by
 * a payload. Loading validates structural fields and CRC before considering a
 * sequence number. Saving always writes the inactive slot first, commits the
 * backend, and only then treats the replacement as current.
 *
 * Sequence comparison uses signed subtraction so 32-bit wraparound remains
 * well-defined as long as fewer than 2^31 saves separate the two records.
 *
 * The store uses two fixed-size slots. Each slot contains a header followed by
 * a payload. Loading validates structural fields and CRC before considering a
 * sequence number. Saving always writes the inactive slot first, commits the
 * backend, and only then treats the replacement as current.
 *
 * Sequence comparison deliberately uses signed subtraction so wraparound of the
 * 32-bit counter still preserves ordering as long as fewer than 2^31 saves
 * separate the two records.
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

#include <array>
#include <cstring>
#include <eurorack/storage/crc32.hpp>
#include <eurorack/storage/record_store.hpp>
#include <vector>

namespace eurorack::storage {

namespace {

constexpr std::size_t SLOT_COUNT = 2U;

bool sequenceNewer(const std::uint32_t candidate, const std::uint32_t reference) noexcept {
    return static_cast<std::int32_t>(candidate - reference) > 0;
}

} // namespace

RecordStore::RecordStore(PersistentStorage& storage, const RecordStoreConfig config) noexcept
    : storage_(storage), config_(config) {}

std::size_t RecordStore::maximumPayloadSize() const noexcept {
    return config_.slotSize > sizeof(RecordHeader) ? config_.slotSize - sizeof(RecordHeader) : 0U;
}

RecordLoadResult RecordStore::load(std::uint8_t* const destination,
                                   const std::size_t destinationCapacity) const noexcept {
    RecordLoadResult result{};

    if (destination == nullptr && destinationCapacity > 0U) {
        result.status = RecordStatus::InvalidArgument;
        return result;
    }

    // Validate both copies independently. A corrupt slot must not prevent the
    // redundant copy from being loaded.
    // Validate both copies independently. A corrupt slot must not prevent the
    // redundant copy from being considered.
    const SlotInfo first = inspectSlot(0U);
    const SlotInfo second = inspectSlot(1U);

    // Compare sequence numbers only after both records passed header and CRC
    // validation. Bytes from invalid media are never trusted as ordering data.
    // Sequence numbers from invalid media are never trusted. Selection starts
    // only after header, payload bounds, and checksum validation succeeded.
    const SlotInfo* selected = nullptr;

    if (first.valid && second.valid) {
        selected = sequenceNewer(second.header.sequence, first.header.sequence) ? &second : &first;
    } else if (first.valid) {
        selected = &first;
    } else if (second.valid) {
        selected = &second;
    }

    if (selected == nullptr) {
        result.status = RecordStatus::NoValidRecord;
        return result;
    }

    result.payloadSize = selected->header.payloadSize;
    result.sequence = selected->header.sequence;
    result.storedSchemaVersion = selected->header.schemaVersion;

    if (selected->header.schemaVersion != config_.schemaVersion) {
        result.status = RecordStatus::VersionMismatch;
        return result;
    }

    if (destinationCapacity < selected->header.payloadSize) {
        result.status = RecordStatus::PayloadTooLarge;
        return result;
    }

    const auto ioResult = storage_.read(
        selected->address + sizeof(RecordHeader), destination, selected->header.payloadSize);

    result.status = ioResult == eurorack::io::IoResult::Success ? RecordStatus::Success
                                                                : RecordStatus::StorageError;

    return result;
}

RecordStatus RecordStore::store(const std::uint8_t* const payload,
                                const std::size_t payloadSize) noexcept {
    if (payload == nullptr && payloadSize > 0U) {
        return RecordStatus::InvalidArgument;
    }

    if (payloadSize > maximumPayloadSize() || payloadSize > 0xFFFFU) {
        return RecordStatus::PayloadTooLarge;
    }

    const SlotInfo first = inspectSlot(0U);
    const SlotInfo second = inspectSlot(1U);

    // Select the inactive or older slot. The still-valid record is preserved
    // until the replacement has been completely written and committed.
    // Preserve the newest valid record while writing the older or empty slot.
    // The backend commit is the synchronization point that makes replacement
    // media-visible.
    std::size_t targetSlot = 0U;
    std::uint32_t nextSequence = 1U;

    if (first.valid && second.valid) {
        const bool secondIsNewer = sequenceNewer(second.header.sequence, first.header.sequence);
        targetSlot = secondIsNewer ? 0U : 1U;
        nextSequence = (secondIsNewer ? second.header.sequence : first.header.sequence) + 1U;
    } else if (first.valid) {
        targetSlot = 1U;
        nextSequence = first.header.sequence + 1U;
    } else if (second.valid) {
        targetSlot = 0U;
        nextSequence = second.header.sequence + 1U;
    }

    const std::size_t address = slotAddress(targetSlot);

    if (storage_.erase(address, config_.slotSize) != eurorack::io::IoResult::Success) {
        return RecordStatus::StorageError;
    }

    if (payloadSize > 0U && storage_.write(address + sizeof(RecordHeader), payload, payloadSize) !=
                                eurorack::io::IoResult::Success) {
        return RecordStatus::StorageError;
    }

    RecordHeader header{};
    header.magic = config_.magic;
    header.schemaVersion = config_.schemaVersion;
    header.payloadSize = static_cast<std::uint16_t>(payloadSize);
    header.sequence = nextSequence;
    header.payloadCrc32 = crc32(payload, payloadSize);
    header.headerCrc32 = calculateHeaderCrc(header);

    if (storage_.write(address, reinterpret_cast<const std::uint8_t*>(&header), sizeof(header)) !=
        eurorack::io::IoResult::Success) {
        return RecordStatus::StorageError;
    }

    return storage_.commit() == eurorack::io::IoResult::Success ? RecordStatus::Success
                                                                : RecordStatus::StorageError;
}

RecordStatus RecordStore::clear() noexcept {
    const std::size_t size = config_.slotSize * SLOT_COUNT;

    if (storage_.erase(config_.baseAddress, size) != eurorack::io::IoResult::Success) {
        return RecordStatus::StorageError;
    }

    return storage_.commit() == eurorack::io::IoResult::Success ? RecordStatus::Success
                                                                : RecordStatus::StorageError;
}

RecordStore::SlotInfo RecordStore::inspectSlot(const std::size_t slotIndex) const noexcept {
    SlotInfo info{};
    info.address = slotAddress(slotIndex);

    if (!readHeader(info.address, info.header) || !validateHeader(info.header)) {
        return info;
    }

    std::vector<std::uint8_t> payload(info.header.payloadSize);

    if (storage_.read(info.address + sizeof(RecordHeader), payload.data(), payload.size()) !=
        eurorack::io::IoResult::Success) {
        return info;
    }

    info.valid = crc32(payload.data(), payload.size()) == info.header.payloadCrc32;

    return info;
}

bool RecordStore::readHeader(const std::size_t address, RecordHeader& header) const noexcept {
    return storage_.read(address, reinterpret_cast<std::uint8_t*>(&header), sizeof(header)) ==
           eurorack::io::IoResult::Success;
}

bool RecordStore::validateHeader(const RecordHeader& header) const noexcept {
    return header.magic == config_.magic && header.payloadSize <= maximumPayloadSize() &&
           header.headerCrc32 == calculateHeaderCrc(header);
}

std::uint32_t RecordStore::calculateHeaderCrc(const RecordHeader& header) const noexcept {
    RecordHeader copy = header;
    copy.headerCrc32 = 0U;

    return crc32(reinterpret_cast<const std::uint8_t*>(&copy), sizeof(copy));
}

std::size_t RecordStore::slotAddress(const std::size_t slotIndex) const noexcept {
    return config_.baseAddress + slotIndex * config_.slotSize;
}

} // namespace eurorack::storage
