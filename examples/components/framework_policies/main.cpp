#include <cstdint>
#include <eurorack/controls/encoder_acceleration.hpp>
#include <eurorack/controls/event_queue.hpp>
#include <eurorack/controls/soft_takeover.hpp>
struct Event { std::uint8_t id; };
int main(){eurorack::controls::EventQueue<Event,4> queue;queue.push({1});Event e{};queue.pop(e);eurorack::controls::SoftTakeover pickup;pickup.arm(512,100);pickup.update(512);eurorack::controls::EncoderAcceleration acceleration;return e.id==1 && pickup.active() && acceleration.apply(1,0)==1 ? 0:1;}
