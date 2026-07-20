/**
 * @file tlc5947.hpp
 * @brief Declares a heap-free TLC5947 PWM LED driver with selectable startup behaviour.
 * @author Axel Napolitano
 * @date 2026
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstddef>
#include <cstdint>
#endif
#include <eurorack/drivers/led/led_driver.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <eurorack/io/time_source.hpp>

namespace eurorack::drivers::led {

/** @brief Visual sequence performed while the TLC5947 is initialized. */
enum class Tlc5947StartupMode : std::uint8_t {
    Silent,     ///< Shift and latch a zero frame without making any LED visible.
    Sequential, ///< Illuminate each logical channel once, in ascending order.
    AllFlash    ///< Illuminate all channels briefly and then clear them.
};

/** @brief Static TLC5947 bus and startup configuration. */
struct Tlc5947Config final {
    std::uint32_t spiClockHertz{8'000'000U}; ///< Requested SPI clock.
    bool enableAfterInitialize{true};        ///< Enable outputs after successful initialization.
    Tlc5947StartupMode startupMode{Tlc5947StartupMode::Silent}; ///< Requested startup sequence.
    std::uint16_t startupBrightness{0xFFFFU}; ///< Logical brightness used by visible startup modes.
    std::uint16_t startupStepMilliseconds{35U};   ///< Hold time per sequential channel.
    std::uint16_t startupFlashMilliseconds{120U}; ///< Hold time for the common flash.
};

/** @brief Heap-free TLC5947 chain driver. */
class Tlc5947 final : public LedBank {
  public:
    static constexpr std::size_t channelsPerDevice = 24U;   ///< Logical channels per device.
    static constexpr std::size_t frameBytesPerDevice = 36U; ///< Packed frame bytes per device.

    /**
     * @brief Constructs a blanked driver around caller-owned storage.
     * @param spi Caller-owned SPI bus.
     * @param latch Caller-owned XLAT output.
     * @param values Caller-owned brightness array.
     * @param valueCapacity Number of entries available in values.
     * @param frame Caller-owned packed-frame array.
     * @param frameCapacity Number of bytes available in frame.
     * @param deviceCount Number of daisy-chained devices.
     * @param outputEnable Optional active-low BLANK/OE output.
     * @param delay Optional blocking delay provider required by visible startup modes.
     * @param config SPI and startup configuration.
     */
    Tlc5947(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& latch,
            std::uint16_t* values,
            std::size_t valueCapacity,
            std::uint8_t* frame,
            std::size_t frameCapacity,
            std::size_t deviceCount,
            eurorack::io::DigitalOutput* outputEnable = nullptr,
            eurorack::io::DelayProvider* delay = nullptr,
            Tlc5947Config config = {}) noexcept;

    /** @brief Reports whether the supplied buffers and startup dependencies are valid. @return True
     * for a usable driver. */
    [[nodiscard]] bool valid() const noexcept;
    /** @brief Initializes the driver and performs the configured startup sequence. @return Success
     * or the first validation/SPI error. */
    eurorack::io::IoResult initialize() noexcept;
    /** @brief Returns the number of logical channels. @return Twenty-four channels per configured
     * device, or zero when invalid. */
    [[nodiscard]] std::size_t channelCount() const noexcept override;
    /** @brief Stores one logical brightness value. @param channel Zero-based logical channel.
     * @param brightness Sixteen-bit logical brightness. @return Success or InvalidArgument. */
    eurorack::io::IoResult setBrightness(std::size_t channel,
                                         std::uint16_t brightness) noexcept override;
    /** @brief Reads one buffered logical brightness. @param channel Zero-based logical channel.
     * @return Buffered value, or zero for an invalid channel. */
    [[nodiscard]] std::uint16_t brightness(std::size_t channel) const noexcept override;
    /** @brief Clears all caller-owned logical brightness entries. @return Success or
     * InvalidArgument. */
    eurorack::io::IoResult clear() noexcept override;
    /** @brief Packs, transfers, and latches one complete frame. @return Success or the first
     * SPI/validation error. */
    eurorack::io::IoResult flush() noexcept override;
    /** @brief Controls the optional active-low BLANK/OE pin. @param enabled True to make latched
     * PWM data visible. */
    void setEnabled(bool enabled) noexcept;

  private:
    /** @brief Packs all logical brightness values into the caller-owned wire-format buffer. */
    void buildFrame() noexcept;
    /** @brief Executes the configured diagnostic startup policy. @return Success or the first SPI
     * error. */
    eurorack::io::IoResult runStartupSequence() noexcept;
    /** @brief Converts milliseconds to the delay-provider unit. @param milliseconds Requested
     * blocking delay. */
    void delayMilliseconds(std::uint16_t milliseconds) noexcept;

    eurorack::io::SpiBus& spi_;          ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& latch_; ///< Active XLAT/latch output.
    eurorack::io::DigitalOutput* oe_;    ///< Optional active-low BLANK/OE output.
    eurorack::io::DelayProvider* delay_; ///< Optional blocking delay provider for visible
                                         ///< startup modes.
    std::uint16_t* values_;   ///< Caller-owned buffer of `channels_` logical brightness values.
    std::uint8_t* frame_;     ///< Caller-owned packed wire-format frame buffer.
    std::size_t channels_;    ///< Logical channel count derived from `deviceCount`.
    std::size_t frameSize_;   ///< Packed frame size in bytes derived from `deviceCount`.
    Tlc5947Config config_{};  ///< SPI clock and startup configuration.
    bool valid_{false};       ///< True when the supplied buffers and dependencies are usable.
    bool initialized_{false}; ///< True once `initialize()` has completed successfully.
};

} // namespace eurorack::drivers::led
