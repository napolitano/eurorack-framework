/**
 * @file src/simulation/virtual_io.cpp
 * @brief Implements virtual digital and analog channels.
 *
 * @details
 * Implements deterministic digital, analog, and bidirectional virtual I/O endpoints.
 * Virtual inputs expose test-controlled values. Virtual outputs record the most recently requested value and transition history. Analog endpoints enforce configured code ranges and can inject explicit operation results. These objects model framework contracts, not electrical timing, impedance, or noise.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#include <algorithm>
#include <eurorack/simulation/virtual_io.hpp>

namespace eurorack::simulation {

VirtualDigitalInput::VirtualDigitalInput(const bool high) noexcept : high_(high) {}

bool VirtualDigitalInput::readHigh() const noexcept {
    return high_;
}

void VirtualDigitalInput::setHigh(const bool high) noexcept {
    high_ = high;
}

VirtualDigitalOutput::VirtualDigitalOutput(const bool high) noexcept : high_(high) {}

void VirtualDigitalOutput::writeHigh(const bool high) noexcept {
    if (high_ != high) {
        ++transitionCount_;
        high_ = high;
    }
}

bool VirtualDigitalOutput::lastWrittenHigh() const noexcept {
    return high_;
}

std::uint32_t VirtualDigitalOutput::transitionCount() const noexcept {
    return transitionCount_;
}

void VirtualBidirectionalPin::setDirection(
    const eurorack::io::BidirectionalDigitalPin::Direction direction) noexcept {
    direction_ = direction;
}

eurorack::io::BidirectionalDigitalPin::Direction
VirtualBidirectionalPin::direction() const noexcept {
    return direction_;
}

bool VirtualBidirectionalPin::readHigh() const noexcept {
    return direction_ == eurorack::io::BidirectionalDigitalPin::Direction::Output ? outputHigh_
                                                                                  : externalHigh_;
}

void VirtualBidirectionalPin::writeHigh(const bool high) noexcept {
    outputHigh_ = high;
}

bool VirtualBidirectionalPin::lastWrittenHigh() const noexcept {
    return outputHigh_;
}

void VirtualBidirectionalPin::setExternalHigh(const bool high) noexcept {
    externalHigh_ = high;
}

VirtualAnalogInput::VirtualAnalogInput(const std::uint32_t maximumCode) noexcept
    : maximumCode_(maximumCode) {}

eurorack::io::AnalogSample VirtualAnalogInput::readRaw() noexcept {
    return {code_, result_};
}

std::uint32_t VirtualAnalogInput::maximumCode() const noexcept {
    return maximumCode_;
}

void VirtualAnalogInput::setCode(const std::uint32_t code) noexcept {
    code_ = std::min(code, maximumCode_);
}

void VirtualAnalogInput::setResult(const eurorack::io::IoResult result) noexcept {
    result_ = result;
}

VirtualAnalogOutput::VirtualAnalogOutput(const std::uint32_t maximumCode) noexcept
    : maximumCode_(maximumCode) {}

eurorack::io::IoResult VirtualAnalogOutput::writeRaw(const std::uint32_t code) noexcept {
    if (code > maximumCode_) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (result_ == eurorack::io::IoResult::Success) {
        code_ = code;
    }

    return result_;
}

std::uint32_t VirtualAnalogOutput::lastWrittenCode() const noexcept {
    return code_;
}

std::uint32_t VirtualAnalogOutput::maximumCode() const noexcept {
    return maximumCode_;
}

void VirtualAnalogOutput::setResult(const eurorack::io::IoResult result) noexcept {
    result_ = result;
}

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
