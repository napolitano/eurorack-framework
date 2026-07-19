/**
 * @file src/drivers/led/tlc5947.cpp
 * @brief Implements the TLC5947 PWM LED driver. *
 * @details
 * Logical 16-bit brightness is reduced to the controller's 12-bit PWM range
 * while building the transfer frame. Channels are serialized in device order
 * and latched only after the complete frame has been shifted, preventing
 * partially updated visible output.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <eurorack/drivers/led/tlc5947.hpp>
namespace eurorack::drivers::led {
Tlc5947::Tlc5947(eurorack::io::SpiBus& spi,
                 eurorack::io::DigitalOutput& latch,
                 const std::size_t count,
                 eurorack::io::DigitalOutput* const oe) noexcept
    : spi_(spi), latch_(latch), oe_(oe), values_(count * 24U, 0U), frame_(count * 36U, 0U) {
    latch_.writeHigh(false);
    setEnabled(true);
}
std::size_t Tlc5947::channelCount() const noexcept {
    return values_.size();
}
eurorack::io::IoResult Tlc5947::setBrightness(const std::size_t c, const std::uint16_t v) noexcept {
    if (c >= channelCount())
        return eurorack::io::IoResult::InvalidArgument;
    values_[c] = v;
    return eurorack::io::IoResult::Success;
}
std::uint16_t Tlc5947::brightness(const std::size_t c) const noexcept {
    return c < channelCount() ? values_[c] : 0U;
}
eurorack::io::IoResult Tlc5947::clear() noexcept {
    std::fill(values_.begin(), values_.end(), 0U);
    return eurorack::io::IoResult::Success;
}
void Tlc5947::setEnabled(const bool enabled) noexcept {
    if (oe_ != nullptr)
        oe_->writeHigh(!enabled);
}
void Tlc5947::buildFrame() noexcept {
    std::fill(frame_.begin(), frame_.end(), 0U);
    std::size_t p = 0U;
    for (std::size_t r = values_.size(); r > 0U; --r) {
        const auto pwm = static_cast<std::uint16_t>(values_[r - 1U] >> 4U);
        for (std::int32_t b = 11; b >= 0; --b) {
            if ((pwm & static_cast<std::uint16_t>(1U << b)) != 0U) {
                const auto bi = p / 8U;
                const auto bit = static_cast<std::uint8_t>(7U - (p % 8U));
                frame_[bi] =
                    static_cast<std::uint8_t>(frame_[bi] | static_cast<std::uint8_t>(1U << bit));
            }
            ++p;
        }
    }
}
eurorack::io::IoResult Tlc5947::flush() noexcept {
    buildFrame();
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
} // namespace eurorack::drivers::led
