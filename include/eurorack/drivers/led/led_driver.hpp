/**
 * @file include/eurorack/drivers/led/led_driver.hpp
 * @brief Declares generic buffered LED-driver interfaces.
 *
 * @details
 * The interfaces separate logical 16-bit brightness values from the resolution
 * and transfer format of a concrete LED controller. Setters normally update a
 * local software buffer; flush() is the explicit synchronization point with the
 * physical device.
 *
 * Implementations are non-owning, synchronous, and not internally synchronized.
 * A brightness value is a logical intensity request, not a promise of optical
 * linearity, current, color consistency, or gamma correction.
 *
 * @ingroup drivers
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <eurorack/io/io_result.hpp>

namespace eurorack::drivers::led {

/**
 * @brief Maximum logical brightness accepted by the generic LED API.
 *
 * Concrete devices with lower resolution scale or truncate this value when
 * building their hardware frame.
 */
inline constexpr std::uint16_t MAX_BRIGHTNESS = 0xFFFFU;

/**
 * @brief Abstract single-channel LED intensity control.
 */
class LedChannel {
  public:
    /** @brief Provides polymorphic destruction. */
    virtual ~LedChannel() = default;

    /**
     * @brief Stores a logical brightness value.
     * @param brightness Brightness from zero, fully off, through
     * MAX_BRIGHTNESS, maximum logical intensity.
     */
    virtual void setBrightness(std::uint16_t brightness) noexcept = 0;

    /**
     * @brief Returns the currently stored logical brightness.
     * @return Value in the inclusive range zero through MAX_BRIGHTNESS.
     */
    [[nodiscard]] virtual std::uint16_t brightness() const noexcept = 0;
};

/**
 * @brief Abstract buffered bank of independently addressable LED channels.
 */
class LedBank {
  public:
    /** @brief Provides polymorphic destruction. */
    virtual ~LedBank() = default;

    /**
     * @brief Returns the number of logical LED channels.
     * @return Zero or the number of valid channel indices.
     */
    [[nodiscard]] virtual std::size_t channelCount() const noexcept = 0;

    /**
     * @brief Stores one channel brightness in the software buffer.
     * @param channel Zero-based logical channel index.
     * @param brightness Logical brightness value.
     * @return Success when the channel exists, otherwise InvalidArgument or a
     * more specific implementation error.
     */
    virtual eurorack::io::IoResult setBrightness(
        std::size_t channel,
        std::uint16_t brightness) noexcept = 0;

    /**
     * @brief Returns one buffered channel brightness.
     * @param channel Zero-based logical channel index.
     * @return Buffered brightness, or zero when the implementation defines an
     * out-of-range read as off.
     */
    [[nodiscard]] virtual std::uint16_t brightness(
        std::size_t channel) const noexcept = 0;

    /**
     * @brief Sets every buffered channel to zero.
     * @return Success when the local buffer is cleared. Physical outputs change
     * only after flush() unless the implementation documents otherwise.
     */
    virtual eurorack::io::IoResult clear() noexcept = 0;

    /**
     * @brief Transfers buffered channel state to the physical controller.
     * @return Success or the first bus or pin-related error encountered.
     */
    virtual eurorack::io::IoResult flush() noexcept = 0;
};

/**
 * @brief Adapts one LedBank channel to the LedChannel interface.
 *
 * @details
 * The view stores a non-owning reference to the bank. It deliberately discards
 * the IoResult returned by LedBank::setBrightness(), because the narrower
 * LedChannel interface has no error channel. Use the bank directly when setter
 * error reporting is required.
 */
class LedBankChannel final : public LedChannel {
  public:
    /**
     * @brief Constructs a view over one bank channel.
     * @param bank Bank that must outlive this view.
     * @param channel Zero-based channel index forwarded to the bank.
     */
    LedBankChannel(LedBank& bank, std::size_t channel) noexcept;

    /**
     * @brief Forwards a logical brightness update to the bank.
     * @param brightness Logical brightness value.
     */
    void setBrightness(std::uint16_t brightness) noexcept override;

    /**
     * @brief Returns the bank's buffered value for this channel.
     * @return Logical brightness reported by the bank.
     */
    [[nodiscard]] std::uint16_t brightness() const noexcept override;

  private:
    LedBank& bank_;         ///< Non-owning bank reference.
    std::size_t channel_;   ///< Forwarded zero-based channel index.
};

} // namespace eurorack::drivers::led
