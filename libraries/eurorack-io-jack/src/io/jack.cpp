/**
 * @file src/io/jack.cpp
 * @brief Implements generic Eurorack jack models.
 *
 * @details
 * Implements switched-jack plug detection and normalization state.
 * The model consumes an already sampled detection contact. It does not read or route the jack
 * signal itself. Transition flags are regenerated on every update, while the stable connected and
 * normalled states remain available until the next sample.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/io/jack.hpp>

namespace eurorack::io {

AnalogInputJack::AnalogInputJack(const JackSignalType signalType,
                                 const eurorack::core::VoltageRange range) noexcept
    : signalType_(signalType), range_(range) {
    snapshot_.signalType = signalType_;
}

void AnalogInputJack::update(const float volts) noexcept {
    const float previous = snapshot_.effectiveVolts;
    snapshot_.rawVolts = volts;
    snapshot_.belowRange = volts < range_.minimumVolts;
    snapshot_.aboveRange = volts > range_.maximumVolts;
    snapshot_.effectiveVolts = range_.clamp(volts);
    snapshot_.changed = snapshot_.effectiveVolts != previous;
}

void AnalogInputJack::setConnection(const ConnectionState connection) noexcept {
    snapshot_.connection = connection;
}

const AnalogJackSnapshot& AnalogInputJack::snapshot() const noexcept {
    return snapshot_;
}

AnalogOutputJack::AnalogOutputJack(const JackSignalType signalType,
                                   const eurorack::core::VoltageRange range) noexcept
    : signalType_(signalType), range_(range) {
    snapshot_.signalType = signalType_;
}

void AnalogOutputJack::setVolts(const float volts) noexcept {
    const float previous = snapshot_.effectiveVolts;
    snapshot_.rawVolts = volts;
    snapshot_.belowRange = volts < range_.minimumVolts;
    snapshot_.aboveRange = volts > range_.maximumVolts;
    snapshot_.effectiveVolts = range_.clamp(volts);
    snapshot_.changed = snapshot_.effectiveVolts != previous;
}

void AnalogOutputJack::setConnection(const ConnectionState connection) noexcept {
    snapshot_.connection = connection;
}

const AnalogJackSnapshot& AnalogOutputJack::snapshot() const noexcept {
    return snapshot_;
}

DigitalInputJack::DigitalInputJack(const JackSignalType signalType,
                                   const float lowMaximum,
                                   const float highMinimum) noexcept
    : lowMaximum_(lowMaximum), highMinimum_(highMinimum) {
    snapshot_.signalType = signalType;
}

void DigitalInputJack::update(const float volts) noexcept {
    snapshot_.rising = false;
    snapshot_.falling = false;
    snapshot_.volts = volts;

    if (!snapshot_.high && volts >= highMinimum_) {
        snapshot_.high = true;
        snapshot_.rising = true;
    } else if (snapshot_.high && volts <= lowMaximum_) {
        snapshot_.high = false;
        snapshot_.falling = true;
    }
}

void DigitalInputJack::setConnection(const ConnectionState connection) noexcept {
    snapshot_.connection = connection;
}

const DigitalJackSnapshot& DigitalInputJack::snapshot() const noexcept {
    return snapshot_;
}

DigitalOutputJack::DigitalOutputJack(const JackSignalType signalType,
                                     const float lowVolts,
                                     const float highVolts) noexcept
    : lowVolts_(lowVolts), highVolts_(highVolts) {
    snapshot_.signalType = signalType;
    snapshot_.volts = lowVolts_;
}

void DigitalOutputJack::setHigh(const bool high) noexcept {
    snapshot_.rising = !snapshot_.high && high;
    snapshot_.falling = snapshot_.high && !high;
    snapshot_.high = high;
    snapshot_.volts = high ? highVolts_ : lowVolts_;
}

void DigitalOutputJack::setConnection(const ConnectionState connection) noexcept {
    snapshot_.connection = connection;
}

const DigitalJackSnapshot& DigitalOutputJack::snapshot() const noexcept {
    return snapshot_;
}

} // namespace eurorack::io
