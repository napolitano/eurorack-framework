#include <cstdint>
#include <eurorack/controls/encoder_acceleration.hpp>
#include <eurorack/controls/event_queue.hpp>
#include <eurorack/controls/soft_takeover.hpp>

struct Event {
    std::uint8_t id;
};

int main() {
    eurorack::controls::EventQueue<Event, 4U> queue;
    queue.push({1U});

    Event event{};
    queue.pop(event);

    eurorack::controls::SoftTakeover pickup;
    pickup.arm(512U, 100U);
    pickup.update(512U);

    eurorack::controls::EncoderAcceleration acceleration;
    const bool accelerationIsNeutral = acceleration.apply(1, 0U) == 1;

    return event.id == 1U && pickup.active() && accelerationIsNeutral ? 0 : 1;
}
