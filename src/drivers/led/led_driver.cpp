/**
 * @file src/drivers/led/led_driver.cpp
 * @brief Implements LED bank channel views.
 * @details
 * Implements the generic single-channel view over a buffered LED bank.
 *
 * The adapter retains a non-owning bank reference and channel index. Brightness writes are
 forwarded to the bank's software buffer; physical synchronization still requires the bank's
 explicit flush operation.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/drivers/led/led_driver.hpp>
namespace eurorack::drivers::led {
LedBankChannel::LedBankChannel(LedBank& bank, const std::size_t channel) noexcept
    : bank_(bank), channel_(channel) {}
void LedBankChannel::setBrightness(const std::uint16_t value) noexcept {
    static_cast<void>(bank_.setBrightness(channel_, value));
}
std::uint16_t LedBankChannel::brightness() const noexcept {
    return bank_.brightness(channel_);
}
} // namespace eurorack::drivers::led
