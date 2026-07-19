#include <array>
#include <cstdint>
#include <cstring>
#include <eurorack/storage/byte_codec.hpp>
#include <eurorack/storage/crc32.hpp>
#include <eurorack/storage/memory_storage.hpp>
#include <eurorack/storage/record_store.hpp>
#include <unity.h>

using namespace eurorack::storage;

namespace {

void test_memory_storage_bounds() {
    MemoryStorage storage(16U);

    std::uint8_t value = 1U;
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(storage.write(16U, &value, 1U)));
}

void test_crc32_known_vector() {
    constexpr std::array<std::uint8_t, 9> data{'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    TEST_ASSERT_EQUAL_HEX32(0xCBF43926U, crc32(data.data(), data.size()));
}

void test_byte_codec_round_trip() {
    std::array<std::uint8_t, 32> bytes{};
    ByteWriter writer(bytes.data(), bytes.size());

    TEST_ASSERT_TRUE(writer.writeU16(0x1234U));
    TEST_ASSERT_TRUE(writer.writeU32(0x89ABCDEFU));
    TEST_ASSERT_TRUE(writer.writeI32(-12345));
    TEST_ASSERT_TRUE(writer.writeFloat(1.25F));

    ByteReader reader(bytes.data(), writer.size());

    std::uint16_t u16 = 0U;
    std::uint32_t u32 = 0U;
    std::int32_t i32 = 0;
    float value = 0.0F;

    TEST_ASSERT_TRUE(reader.readU16(u16));
    TEST_ASSERT_TRUE(reader.readU32(u32));
    TEST_ASSERT_TRUE(reader.readI32(i32));
    TEST_ASSERT_TRUE(reader.readFloat(value));

    TEST_ASSERT_EQUAL_HEX16(0x1234U, u16);
    TEST_ASSERT_EQUAL_HEX32(0x89ABCDEFU, u32);
    TEST_ASSERT_EQUAL_INT32(-12345, i32);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 1.25F, value);
}

void test_record_store_round_trip() {
    MemoryStorage storage(512U);
    RecordStore store(storage, {0U, 256U, 0x12345678U, 1U});

    const std::array<std::uint8_t, 4> payload{1U, 2U, 3U, 4U};

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(store.store(payload.data(), payload.size())));

    std::array<std::uint8_t, 8> destination{};
    const auto result = store.load(destination.data(), destination.size());

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success), static_cast<int>(result.status));
    TEST_ASSERT_EQUAL_UINT32(1U, result.sequence);
    TEST_ASSERT_EQUAL_UINT32(payload.size(), result.payloadSize);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload.data(), destination.data(), payload.size());
}

void test_record_store_uses_newest_valid_slot() {
    MemoryStorage storage(512U);
    RecordStore store(storage, {0U, 256U, 0x12345678U, 1U});

    const std::array<std::uint8_t, 2> first{1U, 1U};
    const std::array<std::uint8_t, 2> second{2U, 2U};

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(store.store(first.data(), first.size())));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(store.store(second.data(), second.size())));

    std::array<std::uint8_t, 2> destination{};
    const auto result = store.load(destination.data(), destination.size());

    TEST_ASSERT_EQUAL_UINT32(2U, result.sequence);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(second.data(), destination.data(), second.size());
}

void test_corrupted_latest_slot_falls_back_to_previous() {
    MemoryStorage storage(512U);
    RecordStore store(storage, {0U, 256U, 0x12345678U, 1U});

    const std::array<std::uint8_t, 2> first{1U, 1U};
    const std::array<std::uint8_t, 2> second{2U, 2U};

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(store.store(first.data(), first.size())));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(store.store(second.data(), second.size())));

    std::uint8_t corrupt = 0xAAU;
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(eurorack::io::IoResult::Success),
        static_cast<int>(storage.write(256U + sizeof(RecordHeader), &corrupt, 1U)));

    std::array<std::uint8_t, 2> destination{};
    const auto result = store.load(destination.data(), destination.size());

    TEST_ASSERT_EQUAL_UINT32(1U, result.sequence);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(first.data(), destination.data(), first.size());
}

void test_version_mismatch_is_reported() {
    MemoryStorage storage(512U);
    RecordStore versionOne(storage, {0U, 256U, 0x12345678U, 1U});
    RecordStore versionTwo(storage, {0U, 256U, 0x12345678U, 2U});

    const std::array<std::uint8_t, 1> payload{7U};

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::Success),
                          static_cast<int>(versionOne.store(payload.data(), payload.size())));

    std::array<std::uint8_t, 4> destination{};
    const auto result = versionTwo.load(destination.data(), destination.size());

    TEST_ASSERT_EQUAL_INT(static_cast<int>(RecordStatus::VersionMismatch),
                          static_cast<int>(result.status));
    TEST_ASSERT_EQUAL_UINT16(1U, result.storedSchemaVersion);
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_memory_storage_bounds);
    RUN_TEST(test_crc32_known_vector);
    RUN_TEST(test_byte_codec_round_trip);
    RUN_TEST(test_record_store_round_trip);
    RUN_TEST(test_record_store_uses_newest_valid_slot);
    RUN_TEST(test_corrupted_latest_slot_falls_back_to_previous);
    RUN_TEST(test_version_mismatch_is_reported);

    return UNITY_END();
}
