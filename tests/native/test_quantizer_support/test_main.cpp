#include <array>
#include <cstdint>
#include <eurorack/controls/analog_button_ladder.hpp>
#include <eurorack/drivers/dac/mcp4922.hpp>
#include <eurorack/drivers/led/tlc5947.hpp>
#include <eurorack/simulation/virtual_buses.hpp>
#include <eurorack/simulation/virtual_io.hpp>
#include <eurorack/storage/fixed_slot.hpp>
#include <eurorack/storage/memory_storage.hpp>
#include <unity.h>

namespace {

class RecordingDelay final : public eurorack::io::DelayProvider {
  public:
    void delayMicroseconds(std::uint32_t microseconds) noexcept override {
        calls_[count_] = microseconds;
        ++count_;
    }
    [[nodiscard]] std::size_t count() const noexcept { return count_; }
    [[nodiscard]] std::uint32_t call(std::size_t index) const noexcept { return calls_[index]; }

  private:
    std::array<std::uint32_t, 32U> calls_{};
    std::size_t count_{0U};
};

void test_mcp4922_unbuffered_and_8mhz() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::dac::Mcp4922 dac(spi, cs);
    dac.setCode(eurorack::drivers::dac::Mcp4922Channel::A, 0x123U);
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(dac.flushChannel(eurorack::drivers::dac::Mcp4922Channel::A)));
    const auto& transfer = spi.transfers()[0];
    TEST_ASSERT_EQUAL_UINT32(8'000'000U, transfer.settings.clockHertz);
    TEST_ASSERT_EQUAL_HEX8(0x31U, transfer.transmitted[0]);
    TEST_ASSERT_EQUAL_HEX8(0x23U, transfer.transmitted[1]);
}

void test_mcp4922_buffered_config() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput cs;
    eurorack::drivers::dac::Mcp4922 dac(
        spi, cs, nullptr,
        {4'000'000U, eurorack::drivers::dac::Mcp4922ReferenceBuffer::Buffered});
    static_cast<void>(dac.flushChannel(eurorack::drivers::dac::Mcp4922Channel::B));
    const auto& transfer = spi.transfers()[0];
    TEST_ASSERT_EQUAL_UINT32(4'000'000U, transfer.settings.clockHertz);
    TEST_ASSERT_EQUAL_HEX8(0xF0U, transfer.transmitted[0]);
}

void test_tlc5947_silent_initialization() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput latch;
    eurorack::simulation::VirtualDigitalOutput blank;
    std::array<std::uint16_t, 24U> values{};
    std::array<std::uint8_t, 36U> frame{};
    eurorack::drivers::led::Tlc5947 led(
        spi, latch, values.data(), values.size(), frame.data(), frame.size(), 1U, &blank);
    TEST_ASSERT_TRUE(blank.lastWrittenHigh());
    TEST_ASSERT_TRUE(led.valid());
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(led.initialize()));
    TEST_ASSERT_FALSE(blank.lastWrittenHigh());
    TEST_ASSERT_EQUAL_UINT32(1U, spi.transfers().size());
    TEST_ASSERT_EQUAL_UINT32(36U, spi.transfers()[0].transmitted.size());
}

void test_tlc5947_sequential_startup() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput latch;
    eurorack::simulation::VirtualDigitalOutput blank;
    RecordingDelay delay;
    std::array<std::uint16_t, 24U> values{};
    std::array<std::uint8_t, 36U> frame{};
    eurorack::drivers::led::Tlc5947Config config{};
    config.startupMode = eurorack::drivers::led::Tlc5947StartupMode::Sequential;
    config.startupStepMilliseconds = 2U;
    eurorack::drivers::led::Tlc5947 led(
        spi, latch, values.data(), values.size(), frame.data(), frame.size(), 1U, &blank, &delay, config);
    TEST_ASSERT_TRUE(led.valid());
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(led.initialize()));
    TEST_ASSERT_EQUAL_UINT32(26U, spi.transfers().size());
    TEST_ASSERT_EQUAL_UINT32(24U, delay.count());
    TEST_ASSERT_EQUAL_UINT32(2'000U, delay.call(0U));
    for (const auto value : values) {
        TEST_ASSERT_EQUAL_UINT16(0U, value);
    }
}

void test_tlc5947_all_flash_startup() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput latch;
    eurorack::simulation::VirtualDigitalOutput blank;
    RecordingDelay delay;
    std::array<std::uint16_t, 24U> values{};
    std::array<std::uint8_t, 36U> frame{};
    eurorack::drivers::led::Tlc5947Config config{};
    config.startupMode = eurorack::drivers::led::Tlc5947StartupMode::AllFlash;
    config.startupFlashMilliseconds = 7U;
    eurorack::drivers::led::Tlc5947 led(
        spi, latch, values.data(), values.size(), frame.data(), frame.size(), 1U, &blank, &delay, config);
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(led.initialize()));
    TEST_ASSERT_EQUAL_UINT32(3U, spi.transfers().size());
    TEST_ASSERT_EQUAL_UINT32(1U, delay.count());
    TEST_ASSERT_EQUAL_UINT32(7'000U, delay.call(0U));
}

void test_tlc5947_visible_mode_requires_delay_provider() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput latch;
    std::array<std::uint16_t, 24U> values{};
    std::array<std::uint8_t, 36U> frame{};
    eurorack::drivers::led::Tlc5947Config config{};
    config.startupMode = eurorack::drivers::led::Tlc5947StartupMode::AllFlash;
    eurorack::drivers::led::Tlc5947 led(
        spi, latch, values.data(), values.size(), frame.data(), frame.size(), 1U, nullptr, nullptr, config);
    TEST_ASSERT_FALSE(led.valid());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(led.initialize()));
}

void test_tlc5947_does_not_latch_failed_transfer() {
    eurorack::simulation::VirtualSpiBus spi;
    eurorack::simulation::VirtualDigitalOutput latch;
    std::array<std::uint16_t, 24U> values{};
    std::array<std::uint8_t, 36U> frame{};
    eurorack::drivers::led::Tlc5947 led(
        spi, latch, values.data(), values.size(), frame.data(), frame.size(), 1U);
    spi.setNextResult(eurorack::io::IoResult::BusError);
    TEST_ASSERT_NOT_EQUAL(0, static_cast<int>(led.flush()));
    TEST_ASSERT_FALSE(latch.lastWrittenHigh());
}

void test_analog_button_ladder_stability() {
    constexpr std::uint16_t codes[3] = {100U, 200U, 300U};
    eurorack::controls::AnalogButtonLadder ladder({codes, 3U, 400U, 10U, 64U});
    ladder.reset(500U, 0U);
    ladder.update(201U, 1U);
    ladder.update(202U, 64U);
    TEST_ASSERT_EQUAL_INT(-1, ladder.snapshot().pressedIndex);
    ladder.update(202U, 65U);
    TEST_ASSERT_EQUAL_INT(1, ladder.snapshot().pressedIndex);
    TEST_ASSERT_EQUAL_INT(1, ladder.snapshot().justPressedIndex);
}

void test_fixed_slot_marker_last_roundtrip() {
    eurorack::storage::MemoryStorage storage(32U);
    eurorack::storage::FixedSlot slot(storage, {4U, 3U, 0xAAU});
    std::uint8_t input[3] = {1U, 2U, 3U};
    std::uint8_t output[3] = {};
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(slot.store(input, 3U)));
    TEST_ASSERT_TRUE(slot.valid());
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(slot.load(output, 3U)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(input, output, 3U);
    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(slot.clear()));
    TEST_ASSERT_FALSE(slot.valid());
}

} // namespace

extern "C" {
void setUp() {}
void tearDown() {}
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_mcp4922_unbuffered_and_8mhz);
    RUN_TEST(test_mcp4922_buffered_config);
    RUN_TEST(test_tlc5947_silent_initialization);
    RUN_TEST(test_tlc5947_sequential_startup);
    RUN_TEST(test_tlc5947_all_flash_startup);
    RUN_TEST(test_tlc5947_visible_mode_requires_delay_provider);
    RUN_TEST(test_tlc5947_does_not_latch_failed_transfer);
    RUN_TEST(test_analog_button_ladder_stability);
    RUN_TEST(test_fixed_slot_marker_last_roundtrip);
    return UNITY_END();
}
