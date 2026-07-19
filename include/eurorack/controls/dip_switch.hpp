/**
 * @file include/eurorack/controls/dip_switch.hpp
 * @brief Declares individual and banked DIP-switch models.
 *
 * @details
 * Represents maintained binary configuration switches and provides allocation-free
 * compile-time-sized banks with bit-mask conversion.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <eurorack/controls/toggle_switch.hpp>

namespace eurorack::controls {

/**
 * @brief One debounced DIP-switch pole.
 *
 * @details
 * DIP switches are electrically equivalent to maintained binary switches but
 * are named separately because they normally encode startup or service
 * configuration rather than live performance gestures.
 */
class DipSwitch final {
  public:
    /** @brief Constructs a DIP-switch pole with default settings. */
    DipSwitch() noexcept = default;

    /** @brief Constructs a DIP-switch pole. @param config Polarity and debounce settings. */
    explicit DipSwitch(ToggleSwitchConfig config) noexcept;

    /** @brief Resets the pole. @param rawLevelHigh Electrical input level. @param nowMs Monotonic
     * milliseconds. */
    void reset(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /** @brief Processes one sample. @param rawLevelHigh Electrical input level. @param nowMs
     * Monotonic milliseconds. */
    void update(bool rawLevelHigh, std::uint32_t nowMs) noexcept;

    /** @brief Reports the stable On state. @return True when On. */
    [[nodiscard]] bool isOn() const noexcept;

    /** @brief Returns the underlying switch snapshot. @return Constant state reference. */
    [[nodiscard]] const ToggleSwitchSnapshot& snapshot() const noexcept;

  private:
    ToggleSwitch switch_{};
};

/**
 * @brief Allocation-free bank of DIP-switch poles.
 * @tparam Count Number of physical poles. Valid range is 1 through 32.
 */
template <std::size_t Count> class DipSwitchBank final {
    static_assert(Count > 0U && Count <= 32U, "DipSwitchBank supports between 1 and 32 poles");

  public:
    /** @brief Constructs every pole with identical settings. @param config Shared pole
     * configuration. */
    explicit DipSwitchBank(const ToggleSwitchConfig config = {}) noexcept {
        switches_.fill(DipSwitch(config));
    }

    /**
     * @brief Resets all poles from a raw-level array.
     * @param rawLevelsHigh Electrical levels indexed from pole zero.
     * @param nowMs Monotonic milliseconds.
     */
    void reset(const std::array<bool, Count>& rawLevelsHigh, const std::uint32_t nowMs) noexcept {
        for (std::size_t index = 0U; index < Count; ++index) {
            switches_[index].reset(rawLevelsHigh[index], nowMs);
        }
    }

    /**
     * @brief Updates all poles from a raw-level array.
     * @param rawLevelsHigh Electrical levels indexed from pole zero.
     * @param nowMs Monotonic milliseconds.
     */
    void update(const std::array<bool, Count>& rawLevelsHigh, const std::uint32_t nowMs) noexcept {
        for (std::size_t index = 0U; index < Count; ++index) {
            switches_[index].update(rawLevelsHigh[index], nowMs);
        }
    }

    /** @brief Returns one pole. @param index Zero-based pole index. @return Constant pole
     * reference. */
    [[nodiscard]] const DipSwitch& switchAt(const std::size_t index) const noexcept {
        return switches_[index < Count ? index : 0U];
    }

    /**
     * @brief Packs stable pole states into a 32-bit mask.
     * @return Mask where bit zero represents pole zero.
     */
    [[nodiscard]] std::uint32_t bitMask() const noexcept {
        std::uint32_t result = 0U;
        for (std::size_t index = 0U; index < Count; ++index) {
            if (switches_[index].isOn()) {
                result |= static_cast<std::uint32_t>(1UL << index);
            }
        }
        return result;
    }

    /** @brief Returns the compile-time pole count. @return Number of poles. */
    [[nodiscard]] static constexpr std::size_t size() noexcept {
        return Count;
    }

  private:
    std::array<DipSwitch, Count> switches_{};
};

} // namespace eurorack::controls
