/**
 * @file include/eurorack/compat/avr/algorithm.hpp
 * @brief Provides the small subset of `<algorithm>` the framework uses, on AVR.
 *
 * @details
 * Covers exactly `min`, `max`, `clamp`, `swap`, `copy`, and `fill`, matching
 * every call site across the framework's implementation files at the time
 * this header was written. All framework call sites use pointer or
 * `std::array`-iterator arguments (never a real iterator category more exotic
 * than a raw pointer), so plain pointer-based templates are sufficient here;
 * this is not a general-purpose `<algorithm>` replacement. Extend this list
 * only when a new framework source needs another function, and only after
 * confirming AVR still lacks it. See `eurorack/compat/avr/cstdint.hpp` for
 * background. Do not include this header directly; include `<algorithm>` as
 * usual.
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
 * @ingroup compat_avr
 */

#pragma once

namespace std {

/**
 * @brief Returns the smaller of two values.
 *
 * @param first First value to compare.
 * @param second Second value to compare.
 * @return `second` if it is smaller than `first`; otherwise `first`.
 */
template <typename T> constexpr const T& min(const T& first, const T& second) noexcept {
    return second < first ? second : first;
}

/**
 * @brief Returns the larger of two values.
 *
 * @param first First value to compare.
 * @param second Second value to compare.
 * @return `second` if it is larger than `first`; otherwise `first`.
 */
template <typename T> constexpr const T& max(const T& first, const T& second) noexcept {
    return first < second ? second : first;
}

/**
 * @brief Constrains a value to an inclusive range.
 *
 * @param value Value to constrain.
 * @param lower Inclusive lower bound.
 * @param upper Inclusive upper bound.
 * @return `lower` when `value` is below it, `upper` when `value` is above it, or `value`
 * unchanged when it already lies inside the range.
 */
template <typename T>
constexpr const T& clamp(const T& value, const T& lower, const T& upper) noexcept {
    return value < lower ? lower : (upper < value ? upper : value);
}

/**
 * @brief Exchanges the values of two objects.
 *
 * @param first First object.
 * @param second Second object.
 */
template <typename T> void swap(T& first, T& second) noexcept {
    T temporary = static_cast<T&&>(first);
    first = static_cast<T&&>(second);
    second = static_cast<T&&>(temporary);
}

/**
 * @brief Copies a range of elements to a destination.
 *
 * @param first Start of the source range.
 * @param last End of the source range, exclusive.
 * @param destination Start of the destination range; must not overlap the source range.
 * @return Iterator one past the last element written.
 */
template <typename InputIt, typename OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt destination) noexcept {
    while (first != last) {
        *destination = *first;
        ++first;
        ++destination;
    }
    return destination;
}

/**
 * @brief Assigns a value to every element in a range.
 *
 * @param first Start of the range.
 * @param last End of the range, exclusive.
 * @param value Value assigned to each element.
 */
template <typename ForwardIt, typename T>
void fill(ForwardIt first, ForwardIt last, const T& value) noexcept {
    while (first != last) {
        *first = value;
        ++first;
    }
}

} // namespace std
