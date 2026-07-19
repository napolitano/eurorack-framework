/**
 * @file src/simulation/virtual_time.cpp
 * @brief Implements deterministic virtual time.
 *
 * @details
 * Advances an internal 64-bit counter and exposes the framework time interfaces.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#include <eurorack/simulation/virtual_time.hpp>

namespace eurorack::simulation {

std::uint32_t VirtualTime::milliseconds() const noexcept {
    return static_cast<std::uint32_t>(microseconds_ / 1000U);
}

std::uint32_t VirtualTime::microseconds() const noexcept {
    return static_cast<std::uint32_t>(microseconds_);
}

void VirtualTime::delayMilliseconds(const std::uint32_t durationMs) noexcept {
    advanceMicroseconds(static_cast<std::uint64_t>(durationMs) * 1000U);
}

void VirtualTime::delayMicroseconds(const std::uint32_t durationUs) noexcept {
    advanceMicroseconds(durationUs);
}

void VirtualTime::setMicroseconds(const std::uint64_t microseconds) noexcept {
    microseconds_ = microseconds;
}

void VirtualTime::advanceMicroseconds(const std::uint64_t microseconds) noexcept {
    microseconds_ += microseconds;
}

std::uint64_t VirtualTime::microseconds64() const noexcept {
    return microseconds_;
}

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
