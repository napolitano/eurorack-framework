/**
 * @file src/storage/fixed_slot.cpp
 * @brief Implements marker-last fixed-slot persistence.
 *
 * @details
 * Writes payload bytes before the validity marker so an interrupted write leaves the slot marked
 * invalid rather than exposing a torn payload as valid.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/storage/fixed_slot.hpp>

namespace eurorack::storage {

eurorack::io::IoResult FixedSlot::store(const std::uint8_t* payload, std::size_t size) noexcept {
    if ((payload == nullptr && size > 0U) || size != config_.payloadSize) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::uint8_t invalid = 0U;
    eurorack::io::IoResult result = storage_.write(config_.address, &invalid, 1U);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    if (size > 0U) {
        result = storage_.write(config_.address + 1U, payload, size);
        if (result != eurorack::io::IoResult::Success) {
            return result;
        }
    }

    result = storage_.write(config_.address, &config_.marker, 1U);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    return storage_.commit();
}

eurorack::io::IoResult FixedSlot::load(std::uint8_t* payload, std::size_t capacity) const noexcept {
    if ((payload == nullptr && capacity > 0U) || capacity < config_.payloadSize) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::uint8_t marker = 0U;
    const eurorack::io::IoResult result = storage_.read(config_.address, &marker, 1U);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    if (marker != config_.marker) {
        return eurorack::io::IoResult::Busy;
    }

    return storage_.read(config_.address + 1U, payload, config_.payloadSize);
}

eurorack::io::IoResult FixedSlot::clear() noexcept {
    std::uint8_t invalid = 0U;
    const eurorack::io::IoResult result = storage_.write(config_.address, &invalid, 1U);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    return storage_.commit();
}

bool FixedSlot::valid() const noexcept {
    std::uint8_t marker = 0U;
    return storage_.read(config_.address, &marker, 1U) == eurorack::io::IoResult::Success &&
           marker == config_.marker;
}

} // namespace eurorack::storage
