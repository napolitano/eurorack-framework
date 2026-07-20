/** @file event_queue.hpp
 * @brief Declares a fixed-capacity allocation-free FIFO event queue.
 */
/**
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#if defined(__AVR__)
#include <eurorack/compat/avr/array.hpp>
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <array>
#include <cstddef>
#endif
namespace eurorack::controls {
/** @brief Fixed-capacity FIFO suitable for foreground event delivery. */
template <typename Event, std::size_t Capacity> class EventQueue final {
  static_assert(Capacity > 0U, "EventQueue capacity must be non-zero");
 public:
  /** @brief Adds an event unless the queue is full. */
  bool push(const Event& event) noexcept { if (size_ == Capacity) { if (dropped_ != static_cast<std::size_t>(-1)) { ++dropped_; } return false; } storage_[(head_+size_)%Capacity]=event; ++size_; return true; }
  /** @brief Removes the oldest event. */
  bool pop(Event& event) noexcept { if (size_==0U) return false; event=storage_[head_]; head_=(head_+1U)%Capacity; --size_; return true; }
  /** @brief Removes all queued events without resetting diagnostics. */
  void clear() noexcept { head_=0U; size_=0U; }
  /** @brief Returns queued event count. */ [[nodiscard]] std::size_t size() const noexcept { return size_; }
  /** @brief Returns queue capacity. */ [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }
  /** @brief Reports whether the queue is empty. */ [[nodiscard]] bool empty() const noexcept { return size_==0U; }
  /** @brief Reports whether the queue is full. */ [[nodiscard]] bool full() const noexcept { return size_==Capacity; }
  /** @brief Returns number of rejected pushes, saturating at size_t maximum. */ [[nodiscard]] std::size_t dropped() const noexcept { return dropped_; }
 private:
  std::array<Event,Capacity> storage_{}; std::size_t head_{0U}; std::size_t size_{0U}; std::size_t dropped_{0U};
};
}
