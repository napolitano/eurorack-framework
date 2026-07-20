/** @file adc_scanner3.hpp @brief Interrupt-driven A5/A6/A7 scanner.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#pragma once
#if !defined(__AVR__)
#error "adc_scanner3.hpp requires an AVR target"
#endif
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <eurorack/compat/avr/cstdint.hpp>
namespace eurorack::platform::avr {
enum class AdcReference : std::uint8_t { ExternalAref, Avcc };
struct AdcScanner3Snapshot final {std::uint16_t a5{0U},a6{0U},a7{0U};bool ready{false};};
class AdcScanner3 final {public:
 /** @brief Performs the documented deterministic operation. */
 /** @brief Initializes free-running conversion. @param reference ADC reference selection. */
 static void initialize(AdcReference reference=AdcReference::Avcc) noexcept;
 /** @brief Atomically reads median-filtered values. @return Current scanner snapshot. */
 static AdcScanner3Snapshot snapshot() noexcept;
 /** @brief Handles one conversion-complete interrupt. */
 static void onConversionComplete() noexcept;
 /** @brief Performs the documented deterministic operation. */
 private:
 /** @brief Returns the median of three codes. @param a First code. @param b Second code. @param c Third code. @return Median code. */
 static std::uint16_t median(std::uint16_t a,std::uint16_t b,std::uint16_t c) noexcept;
 static volatile std::uint16_t samples_[3][3]; static volatile std::uint8_t channel_,sampleIndex_,discard_; static volatile bool ready_;};
} // namespace eurorack::platform::avr
