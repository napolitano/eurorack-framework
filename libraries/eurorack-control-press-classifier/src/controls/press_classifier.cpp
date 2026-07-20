/**
 * @file press_classifier.cpp
 * @brief Implements timestamp-driven short, double, long, and repeat gesture classification.
 * @details The implementation is allocation-free and keeps hardware access explicit through framework interfaces.
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#include <eurorack/controls/press_classifier.hpp>
namespace eurorack::controls {
void PressClassifier::reset(bool pressed, std::uint32_t nowMs) noexcept { pressed_=pressed; longEmitted_=false; pendingShort_=false; pressedAt_=nowMs; releasedAt_=nowMs; nextRepeatAt_=nowMs+config_.repeatDelayMs; }
PressEvent PressClassifier::update(bool pressed, std::uint32_t nowMs) noexcept {
 if (pressed != pressed_) { pressed_=pressed; if (pressed) { pressedAt_=nowMs; longEmitted_=false; nextRepeatAt_=nowMs+config_.repeatDelayMs; return PressEvent::None; }
   if (longEmitted_) return PressEvent::None;
   if (pendingShort_ && !elapsed(nowMs,releasedAt_,config_.doublePressMs)) { pendingShort_=false; return PressEvent::DoublePress; }
   pendingShort_=true; releasedAt_=nowMs; return PressEvent::None; }
 if (pressed_) { if (!longEmitted_ && elapsed(nowMs,pressedAt_,config_.longPressMs)) { longEmitted_=true; nextRepeatAt_=nowMs+config_.repeatIntervalMs; return PressEvent::LongPress; }
   if (longEmitted_ && config_.repeatIntervalMs>0U && reached(nowMs,nextRepeatAt_)) { nextRepeatAt_=nowMs+config_.repeatIntervalMs; return PressEvent::Repeat; }}
 return poll(nowMs);
}
PressEvent PressClassifier::poll(std::uint32_t nowMs) noexcept { if (pendingShort_ && elapsed(nowMs,releasedAt_,config_.doublePressMs)) { pendingShort_=false; return PressEvent::ShortPress; } return PressEvent::None; }
}
