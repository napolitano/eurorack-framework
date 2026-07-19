/**
 * @file src/drivers/led/tlc5916.cpp
 * @brief Implements the TLC5916 LED driver. *
 * @details
 * TLC5916 exposes binary constant-current sink outputs rather than independent
 * PWM channels. The generic 16-bit brightness API is therefore interpreted as
 * off at zero and on for any nonzero value. All outputs are latched together.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <eurorack/drivers/led/tlc5916.hpp>
namespace eurorack::drivers::led {
Tlc5916::Tlc5916(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& latch,
                 const std::size_t count,
                 eurorack::io::DigitalOutput* const oe) noexcept
    : spi_(spi), latch_(latch), oe_(oe), values_(count * 8U, 0U), frame_(count, 0U) {
    latch_.writeHigh(false);
    setEnabled(true);
}
std::size_t Tlc5916::channelCount() const noexcept {
    return values_.size();
}
eurorack::io::IoResult Tlc5916::setBrightness(const std::size_t c, const std::uint16_t v) noexcept {
    if (c >= channelCount())
        return eurorack::io::IoResult::InvalidArgument;
    values_[c] = v;
    return eurorack::io::IoResult::Success;
}
std::uint16_t Tlc5916::brightness(const std::size_t c) const noexcept {
    return c < channelCount() ? values_[c] : 0U;
}
eurorack::io::IoResult Tlc5916::clear() noexcept {
    std::fill(values_.begin(), values_.end(), 0U);
    return eurorack::io::IoResult::Success;
}
eurorack::io::IoResult Tlc5916::flush() noexcept {
    std::fill(frame_.begin(), frame_.end(), 0U);
    for (std::size_t c = 0; c < values_.size(); ++c)
        if (values_[c] != 0U)
            frame_[c / 8U] = static_cast<std::uint8_t>(frame_[c / 8U] |
                                                       static_cast<std::uint8_t>(1U << (c % 8U)));
    const eurorack::io::SpiSettings s{10000000U,
                                      eurorack::io::SpiMode::Mode0,
                                      eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    auto r = spi_.beginTransaction(s);
    if (r != eurorack::io::IoResult::Success)
        return r;
    latch_.writeHigh(false);
    r = spi_.transfer(frame_.data(), nullptr, frame_.size());
    latch_.writeHigh(true);
    latch_.writeHigh(false);
    spi_.endTransaction();
    return r;
}
void Tlc5916::setEnabled(const bool enabled) noexcept {
    if (oe_ != nullptr)
        oe_->writeHigh(!enabled);
}
} // namespace eurorack::drivers::led
