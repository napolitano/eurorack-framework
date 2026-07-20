/** @file fixed_slot.hpp @brief Heap-free fixed-size persistent slot.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
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
struct FixedSlotConfig final { std::size_t address{0U}; std::size_t payloadSize{0U}; std::uint8_t marker{0xAAU}; };
class FixedSlot final {
 public:
  FixedSlot(PersistentStorage& storage,FixedSlotConfig config) noexcept:storage_(storage),config_(config){}
  /** @brief Performs the documented deterministic operation. */
  eurorack::io::IoResult store(const std::uint8_t* payload,std::size_t size) noexcept;
  /** @brief Loads a valid slot. @param payload Destination buffer. @param capacity Destination capacity. @return Operation status. */
  eurorack::io::IoResult load(std::uint8_t* payload,std::size_t capacity) const noexcept;
  /** @brief Invalidates the slot marker. @return Operation status. */
  eurorack::io::IoResult clear() noexcept;
  /** @brief Performs the documented deterministic operation. */
  /** @brief Checks the validity marker. @return True when the marker matches. */
  [[nodiscard]] bool valid() const noexcept;
 private: PersistentStorage& storage_; FixedSlotConfig config_{};
};
} // namespace eurorack::storage
