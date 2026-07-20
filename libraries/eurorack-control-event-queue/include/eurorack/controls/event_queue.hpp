/**
 * @file include/eurorack/controls/event_queue.hpp
 * @brief Declares a fixed-capacity, allocation-free FIFO event queue.
 *
 * @details
 * A generic ring-buffer queue for decoupling event producers (interrupt handlers, polling loops)
 * from event consumers (a foreground dispatch loop) without dynamic allocation. Capacity is a
 * compile-time template parameter; the backing storage is a fixed-size array embedded in the
 * queue object. The class performs no synchronization of its own; if `push` and `pop` can be
 * called from different execution contexts (for example, an interrupt handler and the main
 * loop), the caller is responsible for making each individual call atomic with respect to the
 * other context.
 *
 * @ingroup controls
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
#include <eurorack/compat/avr/array.hpp>
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <array>
#include <cstddef>
#endif

namespace eurorack::controls {

/**
 * @brief Fixed-capacity FIFO queue of `Event` values, suitable for foreground event delivery.
 *
 * @tparam Event Value type stored in the queue; copied on both `push` and `pop`.
 * @tparam Capacity Maximum number of queued events; must be non-zero.
 */
template <typename Event, std::size_t Capacity> class EventQueue final {
    static_assert(Capacity > 0U, "EventQueue capacity must be non-zero");

  public:
    /**
     * @brief Adds an event to the back of the queue unless it is already full.
     *
     * @details
     * When the queue is full, the event is discarded and the drop counter returned by
     * `dropped()` is incremented, saturating rather than wrapping once it reaches the maximum
     * representable count.
     *
     * @param event Event to copy into the queue.
     * @return True if the event was queued; false if the queue was full and the event was
     * dropped.
     */
    bool push(const Event& event) noexcept {
        if (size_ == Capacity) {
            if (dropped_ != static_cast<std::size_t>(-1)) {
                ++dropped_;
            }
            return false;
        }

        storage_[(head_ + size_) % Capacity] = event;
        ++size_;
        return true;
    }

    /**
     * @brief Removes and returns the oldest queued event.
     *
     * @param event Receives the removed event on success; left unchanged if the queue was empty.
     * @return True if an event was removed; false if the queue was empty.
     */
    bool pop(Event& event) noexcept {
        if (size_ == 0U) {
            return false;
        }

        event = storage_[head_];
        head_ = (head_ + 1U) % Capacity;
        --size_;
        return true;
    }

    /**
     * @brief Removes all queued events.
     *
     * @details
     * Does not reset the count returned by `dropped()`; that diagnostic persists across `clear`
     * calls.
     */
    void clear() noexcept {
        head_ = 0U;
        size_ = 0U;
    }

    /**
     * @brief Returns the number of events currently queued.
     *
     * @return Queued event count, between 0 and `capacity()` inclusive.
     */
    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Returns the maximum number of events the queue can hold.
     *
     * @return `Capacity`.
     */
    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    /**
     * @brief Reports whether the queue currently holds no events.
     *
     * @return True when `size() == 0`.
     */
    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0U;
    }

    /**
     * @brief Reports whether the queue is at capacity.
     *
     * @return True when `size() == capacity()`.
     */
    [[nodiscard]] bool full() const noexcept {
        return size_ == Capacity;
    }

    /**
     * @brief Returns the number of events rejected by `push` because the queue was full.
     *
     * @return Rejected-push count since construction or the last object lifetime began; does not
     * reset on `clear()`. Saturates at the maximum representable `std::size_t` value rather than
     * wrapping.
     */
    [[nodiscard]] std::size_t dropped() const noexcept {
        return dropped_;
    }

  private:
    std::array<Event, Capacity> storage_{}; ///< Backing storage, indexed as a circular buffer.
    std::size_t head_{0U};                  ///< Index of the oldest queued event within `storage_`.
    std::size_t size_{0U};                  ///< Number of events currently queued.
    std::size_t dropped_{0U};               ///< Saturating count of events rejected by `push`.
};

} // namespace eurorack::controls
