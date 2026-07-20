/**
 * @file main.cpp
 * @brief Native root-project smoke build for PlatformIO.
 *
 * This executable intentionally exercises a small platform-independent subset
 * of the framework. It gives contributors a deterministic `pio run` command
 * from the repository root without pretending to validate an embedded target.
 */

#include <cstdint>

#include <eurorack/controls/event_queue.hpp>
#include <eurorack/core/framework_config.hpp>

namespace {

struct SmokeEvent final {
    std::uint8_t value{0U};
};

}  // namespace

int main() {
    const eurorack::core::FrameworkConfig configuration{};
    if (!eurorack::core::isValid(configuration)) {
        return 1;
    }

    eurorack::controls::EventQueue<SmokeEvent, 2U> events{};
    if (!events.push(SmokeEvent{42U})) {
        return 2;
    }

    SmokeEvent event{};
    if (!events.pop(event)) {
        return 3;
    }

    return event.value == 42U ? 0 : 4;
}
