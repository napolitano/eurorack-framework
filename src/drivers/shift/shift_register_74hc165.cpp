/**
 * @file src/drivers/shift/shift_register_74hc165.cpp
 * @brief Implements the 74HC165 driver.
 * @details
 * Implements buffered input capture from a chain of 74HC165 parallel-in shift registers.
 *
 * A load pulse snapshots all parallel inputs before SPI shifting begins. Bytes are read in chain
 order into fixed software storage, preventing later pin changes from altering the captured sample.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/drivers/shift/shift_register_74hc165.hpp>
namespace eurorack::drivers::shift {
ShiftRegister74Hc165::ShiftRegister74Hc165(eurorack::io::SpiBus& spi,
                                           eurorack::io::DigitalOutput& load,
                                           const std::size_t count) noexcept
    : spi_(spi), load_(load), buffer_(count, 0U) {
    load_.writeHigh(true);
}
std::size_t ShiftRegister74Hc165::inputCount() const noexcept {
    return buffer_.size() * 8U;
}
eurorack::io::IoResult ShiftRegister74Hc165::sample() noexcept {
    load_.writeHigh(false);
    load_.writeHigh(true);
    const eurorack::io::SpiSettings s{10000000U,
                                      eurorack::io::SpiMode::Mode0,
                                      eurorack::io::SpiBitOrder::MostSignificantBitFirst};
    auto r = spi_.beginTransaction(s);
    if (r != eurorack::io::IoResult::Success)
        return r;
    r = spi_.transfer(nullptr, buffer_.data(), buffer_.size());
    spi_.endTransaction();
    return r;
}
bool ShiftRegister74Hc165::input(const std::size_t n) const noexcept {
    return n < inputCount() && (buffer_[n / 8U] & static_cast<std::uint8_t>(1U << (n % 8U))) != 0U;
}
} // namespace eurorack::drivers::shift
