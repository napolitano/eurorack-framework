#include <array>
#include <cstddef>
#include <cstdint>
#include <eurorack/drivers/display/ssd1306.hpp>
#include <unity.h>
#include <vector>

namespace {

class FakeI2c final : public eurorack::io::I2cBus {
  public:
    eurorack::io::IoResult setClock(const std::uint32_t frequencyHz) noexcept override {
        clockHz = frequencyHz;
        return eurorack::io::IoResult::Success;
    }

    eurorack::io::IoResult write(const eurorack::io::I2cAddress address,
                                 const std::uint8_t* data,
                                 const std::size_t size,
                                 const bool = true) noexcept override {
        lastAddress = address;
        writes.emplace_back(data, data + size);
        return eurorack::io::IoResult::Success;
    }

    eurorack::io::IoResult
    read(const eurorack::io::I2cAddress, std::uint8_t*, const std::size_t) noexcept override {
        return eurorack::io::IoResult::NotSupported;
    }

    eurorack::io::IoResult writeRead(const eurorack::io::I2cAddress,
                                     const std::uint8_t*,
                                     const std::size_t,
                                     std::uint8_t*,
                                     const std::size_t) noexcept override {
        return eurorack::io::IoResult::NotSupported;
    }

    std::uint32_t clockHz{0U};
    eurorack::io::I2cAddress lastAddress{0U};
    std::vector<std::vector<std::uint8_t>> writes{};
};

void test_initialize_sets_clock_and_display_on() {
    FakeI2c bus;
    eurorack::drivers::display::Ssd1306 display(bus);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(display.initialize()));

    TEST_ASSERT_EQUAL_UINT32(400000U, bus.clockHz);
    TEST_ASSERT_TRUE(bus.writes.size() >= 3U);
    TEST_ASSERT_EQUAL_HEX8(0xAFU, bus.writes.back().back());
}

void test_flush_rejects_wrong_canvas_size() {
    FakeI2c bus;
    eurorack::drivers::display::Ssd1306 display(bus);
    static_cast<void>(display.initialize());

    std::array<std::uint8_t, 64> buffer{};
    eurorack::display::MonochromeCanvas canvas(buffer.data(), buffer.size(), 32, 16);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(display.flush(canvas)));
}

void test_set_contrast_writes_command_pair() {
    FakeI2c bus;
    eurorack::drivers::display::Ssd1306 display(bus);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(display.setContrast(0x22U)));

    const auto& packet = bus.writes.back();
    TEST_ASSERT_EQUAL_HEX8(0x00U, packet[0]);
    TEST_ASSERT_EQUAL_HEX8(0x81U, packet[1]);
    TEST_ASSERT_EQUAL_HEX8(0x22U, packet[2]);
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_initialize_sets_clock_and_display_on);
    RUN_TEST(test_flush_rejects_wrong_canvas_size);
    RUN_TEST(test_set_contrast_writes_command_pair);

    return UNITY_END();
}
