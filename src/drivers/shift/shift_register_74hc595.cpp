/**
 * @file src/drivers/shift/shift_register_74hc595.cpp
 * @brief Implements the 74HC595 driver. *
 * @details
 * One buffered byte is stored per physical register. flush() shifts the
 * farthest device first and pulses the latch only after the complete chain has
 * been transferred. Optional output-enable control is expressed in logical
 * enabled terms rather than raw pin polarity.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <eurorack/drivers/shift/shift_register_74hc595.hpp>
namespace eurorack::drivers::shift {
ShiftRegister74Hc595::ShiftRegister74Hc595(eurorack::io::SpiBus& spi,
                                           eurorack::io::DigitalOutput& latch,
                                           const std::size_t count,
                                           eurorack::io::DigitalOutput* const oe) noexcept
    : spi_(spi), latch_(latch), outputEnable_(oe), buffer_(count, 0U) {
    latch_.writeHigh(false);
    setEnabled(true);
}
std::size_t ShiftRegister74Hc595::outputCount() const noexcept {
    return buffer_.size() * 8U;
}
eurorack::io::IoResult ShiftRegister74Hc595::setOutput(const std::size_t n,
                                                       const bool high) noexcept {
    if (n >= outputCount())
        return eurorack::io::IoResult::InvalidArgument;
    const auto i = n / 8U;
    const auto m = static_cast<std::uint8_t>(1U << (n % 8U));
    buffer_[i] = high ? static_cast<std::uint8_t>(buffer_[i] | m)
                      : static_cast<std::uint8_t>(buffer_[i] & static_cast<std::uint8_t>(~m));
    return eurorack::io::IoResult::Success;
}
bool ShiftRegister74Hc595::output(const std::size_t n) const noexcept {
    return n < outputCount() && (buffer_[n / 8U] & static_cast<std::uint8_t>(1U << (n % 8U))) != 0U;
}
eurorack::io::IoResult ShiftRegister74Hc595::clear() noexcept {
    std::fill(buffer_.begin(), buffer_.end(), 0U);
    return eurorack::io::IoResult::Success;
}
eurorack::io::IoResult ShiftRegister74Hc595::flush() noexcept {
    const eurorack::io::SpiSettings s{10000000U,
                                      eurorack::io::SpiMode::Mode0,
                                      eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    auto r = spi_.beginTransaction(s);
    if (r != eurorack::io::IoResult::Success)
        return r;
    latch_.writeHigh(false);
    r = spi_.transfer(buffer_.data(), nullptr, buffer_.size());
    latch_.writeHigh(true);
    latch_.writeHigh(false);
    spi_.endTransaction();
    return r;
}
void ShiftRegister74Hc595::setEnabled(const bool enabled) noexcept {
    if (outputEnable_ != nullptr)
        outputEnable_->writeHigh(!enabled);
}
} // namespace eurorack::drivers::shift
