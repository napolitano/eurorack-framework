/** @file adc_scanner3.cpp @brief Implements the A5/A6/A7 ADC scanner.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 
 * @details Owns the ADC conversion-complete ISR and rotates A5, A6, and A7 with median-of-three snapshots.*/
#include <eurorack/platform/avr/adc_scanner3.hpp>
namespace eurorack::platform::avr {
volatile std::uint16_t AdcScanner3::samples_[3][3]={{0U}};volatile std::uint8_t AdcScanner3::channel_=0U;volatile std::uint8_t AdcScanner3::sampleIndex_=0U;volatile std::uint8_t AdcScanner3::discard_=1U;volatile bool AdcScanner3::ready_=false;
void AdcScanner3::initialize(AdcReference ref) noexcept {ATOMIC_BLOCK(ATOMIC_RESTORESTATE){channel_=0U;sampleIndex_=0U;discard_=1U;ready_=false;ADMUX=static_cast<std::uint8_t>((ref==AdcReference::Avcc?_BV(REFS0):0U)|5U);ADCSRA=_BV(ADEN)|_BV(ADATE)|_BV(ADIE)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);ADCSRB=0U;ADCSRA|=_BV(ADSC);}}
std::uint16_t AdcScanner3::median(std::uint16_t a,std::uint16_t b,std::uint16_t c) noexcept {if(a>b){auto t=a;a=b;b=t;}if(b>c){auto t=b;b=c;c=t;}if(a>b){auto t=a;a=b;b=t;}return b;}
AdcScanner3Snapshot AdcScanner3::snapshot() noexcept {AdcScanner3Snapshot s;ATOMIC_BLOCK(ATOMIC_RESTORESTATE){s.a5=median(samples_[0][0],samples_[0][1],samples_[0][2]);s.a6=median(samples_[1][0],samples_[1][1],samples_[1][2]);s.a7=median(samples_[2][0],samples_[2][1],samples_[2][2]);s.ready=ready_;}return s;}
void AdcScanner3::onConversionComplete() noexcept {auto value=ADC;if(discard_>0U){discard_=0U;return;}samples_[channel_][sampleIndex_]=value;++sampleIndex_;if(sampleIndex_>=3U){sampleIndex_=0U;++channel_;if(channel_>=3U){channel_=0U;ready_=true;}ADMUX=static_cast<std::uint8_t>((ADMUX&0xF0U)|static_cast<std::uint8_t>(5U+channel_));discard_=1U;}}
} // namespace eurorack::platform::avr
ISR(ADC_vect){eurorack::platform::avr::AdcScanner3::onConversionComplete();}
