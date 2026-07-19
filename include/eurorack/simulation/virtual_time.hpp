/**
 * @file include/eurorack/simulation/virtual_time.hpp
 * @brief Declares deterministic virtual time for simulation and native tests.
 *
 * @details
 * Implements monotonic time and non-blocking delay advancement without wall-clock dependencies.
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
 * @ingroup simulation
 */

#if !defined(ARDUINO)

#pragma once

#include <cstdint>
#include <eurorack/io/time_source.hpp>

namespace eurorack::simulation {

/**
 * @brief Deterministic virtual time source and delay provider.
 */
class VirtualTime final : public eurorack::io::TimeSource, public eurorack::io::DelayProvider {
  public:
    /**
     * @brief Constructs virtual time at zero.
     */
    VirtualTime() noexcept = default;

    /**
     * @brief Returns elapsed milliseconds.
     *
     * @return Monotonic milliseconds.
     */
    [[nodiscard]] std::uint32_t milliseconds() const noexcept override;

    /**
     * @brief Returns elapsed microseconds.
     *
     * @return Monotonic microseconds.
     */
    [[nodiscard]] std::uint32_t microseconds() const noexcept override;

    /**
     * @brief Advances virtual time by milliseconds.
     *
     * @param durationMs Milliseconds to advance.
     */
    void delayMilliseconds(std::uint32_t durationMs) noexcept;

    /**
     * @brief Advances virtual time by microseconds.
     *
     * @param durationUs Microseconds to advance.
     */
    void delayMicroseconds(std::uint32_t durationUs) noexcept override;

    /**
     * @brief Sets absolute virtual time.
     *
     * @param microseconds Absolute microsecond value.
     */
    void setMicroseconds(std::uint64_t microseconds) noexcept;

    /**
     * @brief Advances virtual time by an arbitrary amount.
     *
     * @param microseconds Microseconds to add.
     */
    void advanceMicroseconds(std::uint64_t microseconds) noexcept;

    /**
     * @brief Returns the full 64-bit virtual time.
     *
     * @return Elapsed microseconds.
     */
    [[nodiscard]] std::uint64_t microseconds64() const noexcept;

  private:
    std::uint64_t microseconds_{0U}; ///< Full-precision elapsed virtual time. `microseconds()`
                                     ///< returns this value truncated to 32 bits;
                                     ///< `milliseconds()` returns it divided by 1000 and then
                                     ///< truncated to 32 bits.
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
