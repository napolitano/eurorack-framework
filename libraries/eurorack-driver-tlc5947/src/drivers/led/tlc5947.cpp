/**
 * @file tlc5947.cpp
 * @brief Implements the heap-free TLC5947 driver and its startup sequences.
 * @details The implementation keeps outputs blanked until a complete zero frame has been shifted
 * and latched. Optional visible startup diagnostics use the injected delay service, remain outside
 * the real-time path, clear their temporary values before returning, and preserve BLANK on errors.
 * Frame packing serializes daisy-chained devices in reverse logical-channel order as required by
 * the TLC5947 shift-register protocol. No dynamic storage is allocated by this translation unit.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#include <eurorack/drivers/led/tlc5947.hpp>

namespace eurorack::drivers::led {

Tlc5947::Tlc5947(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& latch,
                 std::uint16_t* values,
                 std::size_t valueCapacity,
                 std::uint8_t* frame,
                 std::size_t frameCapacity,
                 std::size_t devices,
                 eurorack::io::DigitalOutput* oe,
                 eurorack::io::DelayProvider* delay,
                 Tlc5947Config config) noexcept
    : spi_(spi), latch_(latch), oe_(oe), delay_(delay), values_(values), frame_(frame),
      channels_(devices * channelsPerDevice), frameSize_(devices * frameBytesPerDevice),
      config_(config) {
    valid_ = devices > 0U && values_ != nullptr && frame_ != nullptr &&
             valueCapacity >= channels_ && frameCapacity >= frameSize_;
    if (config_.startupMode != Tlc5947StartupMode::Silent && delay_ == nullptr) {
        valid_ = false;
    }
    latch_.writeHigh(false);
    setEnabled(false);
    if (valid_) {
        static_cast<void>(clear());
    }
}

bool Tlc5947::valid() const noexcept {
    return valid_;
}

void Tlc5947::delayMilliseconds(std::uint16_t milliseconds) noexcept {
    if (delay_ != nullptr) {
        delay_->delayMicroseconds(static_cast<std::uint32_t>(milliseconds) * 1000U);
    }
}

eurorack::io::IoResult Tlc5947::runStartupSequence() noexcept {
    if (config_.startupMode == Tlc5947StartupMode::Silent) {
        return eurorack::io::IoResult::Success;
    }

    setEnabled(true);
    if (config_.startupMode == Tlc5947StartupMode::Sequential) {
        for (std::size_t channel = 0U; channel < channels_; ++channel) {
            static_cast<void>(clear());
            values_[channel] = config_.startupBrightness;
            const auto result = flush();
            if (result != eurorack::io::IoResult::Success) {
                setEnabled(false);
                return result;
            }
            delayMilliseconds(config_.startupStepMilliseconds);
        }
    } else {
        for (std::size_t channel = 0U; channel < channels_; ++channel) {
            values_[channel] = config_.startupBrightness;
        }
        const auto result = flush();
        if (result != eurorack::io::IoResult::Success) {
            setEnabled(false);
            return result;
        }
        delayMilliseconds(config_.startupFlashMilliseconds);
    }

    static_cast<void>(clear());
    const auto clearResult = flush();
    if (clearResult != eurorack::io::IoResult::Success) {
        setEnabled(false);
    }
    return clearResult;
}

eurorack::io::IoResult Tlc5947::initialize() noexcept {
    if (!valid_) {
        return eurorack::io::IoResult::InvalidArgument;
    }
    setEnabled(false);
    auto result = clear();
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }
    result = flush();
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }
    result = runStartupSequence();
    if (result == eurorack::io::IoResult::Success) {
        initialized_ = true;
        setEnabled(config_.enableAfterInitialize);
    }
    return result;
}

std::size_t Tlc5947::channelCount() const noexcept {
    return valid_ ? channels_ : 0U;
}

eurorack::io::IoResult Tlc5947::setBrightness(std::size_t channel, std::uint16_t value) noexcept {
    if (!valid_ || channel >= channels_) {
        return eurorack::io::IoResult::InvalidArgument;
    }
    values_[channel] = value;
    return eurorack::io::IoResult::Success;
}

std::uint16_t Tlc5947::brightness(std::size_t channel) const noexcept {
    return valid_ && channel < channels_ ? values_[channel] : 0U;
}

eurorack::io::IoResult Tlc5947::clear() noexcept {
    if (!valid_) {
        return eurorack::io::IoResult::InvalidArgument;
    }
    for (std::size_t index = 0U; index < channels_; ++index) {
        values_[index] = 0U;
    }
    return eurorack::io::IoResult::Success;
}

void Tlc5947::setEnabled(bool enabled) noexcept {
    if (oe_ != nullptr) {
        oe_->writeHigh(!enabled);
    }
}

void Tlc5947::buildFrame() noexcept {
    for (std::size_t index = 0U; index < frameSize_; ++index) {
        frame_[index] = 0U;
    }
    std::size_t bitPosition = 0U;
    for (std::size_t reverse = channels_; reverse > 0U; --reverse) {
        const auto pwm = static_cast<std::uint16_t>(values_[reverse - 1U] >> 4U);
        for (std::int8_t bit = 11; bit >= 0; --bit) {
            if ((pwm & static_cast<std::uint16_t>(1U << bit)) != 0U) {
                const auto byteIndex = bitPosition / 8U;
                const auto frameBit = static_cast<std::uint8_t>(7U - (bitPosition % 8U));
                frame_[byteIndex] = static_cast<std::uint8_t>(
                    frame_[byteIndex] | static_cast<std::uint8_t>(1U << frameBit));
            }
            ++bitPosition;
        }
    }
}

eurorack::io::IoResult Tlc5947::flush() noexcept {
    if (!valid_) {
        return eurorack::io::IoResult::InvalidArgument;
    }
    buildFrame();
    const eurorack::io::SpiSettings settings{config_.spiClockHertz,
                                             eurorack::io::SpiMode::Mode0,
                                             eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    auto result = spi_.beginTransaction(settings);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }
    latch_.writeHigh(false);
    result = spi_.transfer(frame_, nullptr, frameSize_);
    if (result == eurorack::io::IoResult::Success) {
        latch_.writeHigh(true);
        latch_.writeHigh(false);
    }
    spi_.endTransaction();
    return result;
}

} // namespace eurorack::drivers::led
