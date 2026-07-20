/**
 * @file include/eurorack/drivers/led/tlc5916.hpp
 * @brief Declares a buffered driver for cascaded TLC5916 constant-current sinks.
 *
 * @details
 * Each TLC5916 contributes eight binary output channels. The generic 16-bit
 * brightness buffer is interpreted as off when zero and on when nonzero because
 * the TLC5916 has no per-channel PWM register. Cascaded devices are transferred
 * as one SPI frame and latched together.
 *
 * The driver allocates its buffers during construction. Avoid constructing it
 * repeatedly in embedded firmware. The SPI bus and pin outputs are non-owning
 * dependencies and must outlive the driver.
 *
 * @ingroup drivers
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
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/drivers/led/led_driver.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <vector>

namespace eurorack::drivers::led {

/**
 * @brief Buffered TLC5916 chain exposed as a generic LedBank.
 */
class Tlc5916 final : public LedBank {
  public:
    /**
     * @brief Constructs a driver for one or more cascaded devices.
     * @param spi SPI bus that must outlive the driver.
     * @param latch Active-high latch output that must outlive the driver.
     * @param deviceCount Number of cascaded TLC5916 devices. Each contributes
     * eight logical channels.
     * @param outputEnable Optional active-low OE output. Pass nullptr when OE is
     * tied active or managed externally.
     */
    Tlc5916(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& latch,
            std::size_t deviceCount,
            eurorack::io::DigitalOutput* outputEnable = nullptr) noexcept;

    /** @brief Returns eight times the configured device count. @return Logical channel count. */
    [[nodiscard]] std::size_t channelCount() const noexcept override;

    /**
     * @brief Stores a logical channel value.
     * @param channel Zero-based channel index.
     * @param brightness Zero selects off; every nonzero value selects on.
     * @return Success or InvalidArgument when the channel is out of range.
     */
    eurorack::io::IoResult setBrightness(std::size_t channel,
                                         std::uint16_t brightness) noexcept override;

    /**
     * @brief Returns one buffered logical value.
     * @param channel Zero-based channel index.
     * @return Buffered value, or zero when the channel is out of range.
     */
    [[nodiscard]] std::uint16_t brightness(std::size_t channel) const noexcept override;

    /**
     * @brief Clears every buffered channel without transferring the frame.
     * @return Success.
     */
    eurorack::io::IoResult clear() noexcept override;

    /**
     * @brief Builds and transfers the binary output frame, then pulses latch.
     * @return Success or the SPI transaction error.
     */
    eurorack::io::IoResult flush() noexcept override;

    /**
     * @brief Controls the optional active-low output-enable pin.
     * @param enabled True to enable sink outputs, false to disable them.
     */
    void setEnabled(bool enabled) noexcept;

  private:
    eurorack::io::SpiBus& spi_;          ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& latch_; ///< Latch output.
    eurorack::io::DigitalOutput* oe_;    ///< Optional active-low OE output.
    std::vector<std::uint16_t> values_;  ///< Logical channel values.
    std::vector<std::uint8_t> frame_;    ///< Packed binary device frame.
};

} // namespace eurorack::drivers::led
