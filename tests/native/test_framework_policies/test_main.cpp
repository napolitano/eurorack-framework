#include <cstdint>
#include <eurorack/controls/encoder_acceleration.hpp>
#include <eurorack/controls/event_queue.hpp>
#include <eurorack/controls/press_classifier.hpp>
#include <eurorack/controls/soft_takeover.hpp>
#include <unity.h>
namespace { struct E{int v;};
void queue_fifo_and_overflow(){eurorack::controls::EventQueue<E,2> q;TEST_ASSERT_TRUE(q.push({1}));TEST_ASSERT_TRUE(q.push({2}));TEST_ASSERT_FALSE(q.push({3}));E e{};TEST_ASSERT_TRUE(q.pop(e));TEST_ASSERT_EQUAL_INT(1,e.v);TEST_ASSERT_EQUAL_UINT32(1,q.dropped());}
void takeover_crossing(){eurorack::controls::SoftTakeover s(2);s.arm(100,20);TEST_ASSERT_FALSE(s.update(90));TEST_ASSERT_TRUE(s.update(101));}
void acceleration_thresholds(){eurorack::controls::EncoderAcceleration a;TEST_ASSERT_EQUAL_INT(1,a.apply(1,100));TEST_ASSERT_EQUAL_INT(8,a.apply(1,110));TEST_ASSERT_EQUAL_INT(3,a.apply(1,150));TEST_ASSERT_EQUAL_INT(1,a.apply(1,300));}
void press_short_double_long(){eurorack::controls::PressClassifier p({100,50,150,20});p.reset(false,0);TEST_ASSERT_EQUAL_INT(0,(int)p.update(true,1));TEST_ASSERT_EQUAL_INT(0,(int)p.update(false,20));TEST_ASSERT_EQUAL_INT((int)eurorack::controls::PressEvent::ShortPress,(int)p.poll(71));p.reset(false,100);p.update(true,101);p.update(false,110);p.update(true,130);TEST_ASSERT_EQUAL_INT((int)eurorack::controls::PressEvent::DoublePress,(int)p.update(false,140));p.reset(false,200);p.update(true,201);TEST_ASSERT_EQUAL_INT((int)eurorack::controls::PressEvent::LongPress,(int)p.update(true,301));}
}
extern "C" {void setUp(){} void tearDown(){}} int main(){UNITY_BEGIN();RUN_TEST(queue_fifo_and_overflow);RUN_TEST(takeover_crossing);RUN_TEST(acceleration_thresholds);RUN_TEST(press_short_double_long);return UNITY_END();}
