#include <array>
#include <cstdint>
#include <eurorack/storage/file_storage.hpp>
#include <filesystem>
#include <unity.h>

namespace {

std::filesystem::path testPath() {
    return std::filesystem::temp_directory_path() / "eurorack_framework_file_storage_test.bin";
}

void removeTestFiles() {
    std::error_code error;
    std::filesystem::remove(testPath(), error);
    std::filesystem::remove(testPath().string() + ".tmp", error);
}

void test_missing_file_starts_erased() {
    removeTestFiles();

    eurorack::storage::FileStorage storage(testPath().string(), 16U);

    TEST_ASSERT_TRUE(storage.ready());

    std::array<std::uint8_t, 4> bytes{};
    static_cast<void>(storage.read(0U, bytes.data(), bytes.size()));

    for (const auto value : bytes) {
        TEST_ASSERT_EQUAL_HEX8(0xFFU, value);
    }
}

void test_commit_and_reopen_preserves_bytes() {
    removeTestFiles();

    {
        eurorack::storage::FileStorage storage(testPath().string(), 16U);

        const std::array<std::uint8_t, 4> source{1U, 2U, 3U, 4U};

        TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                              static_cast<int>(storage.write(3U, source.data(), source.size())));
        TEST_ASSERT_TRUE(storage.dirty());
        TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                              static_cast<int>(storage.commit()));
        TEST_ASSERT_FALSE(storage.dirty());
    }

    eurorack::storage::FileStorage reopened(testPath().string(), 16U);

    std::array<std::uint8_t, 4> destination{};
    static_cast<void>(reopened.read(3U, destination.data(), destination.size()));

    const std::array<std::uint8_t, 4> expected{1U, 2U, 3U, 4U};

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), destination.data(), expected.size());
}

void test_erase_marks_storage_dirty() {
    removeTestFiles();

    eurorack::storage::FileStorage storage(testPath().string(), 8U);

    const std::uint8_t value = 0x12U;
    static_cast<void>(storage.write(0U, &value, 1U));
    static_cast<void>(storage.commit());

    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::Success),
                          static_cast<int>(storage.erase(0U, 1U)));
    TEST_ASSERT_TRUE(storage.dirty());

    std::uint8_t result = 0U;
    static_cast<void>(storage.read(0U, &result, 1U));
    TEST_ASSERT_EQUAL_HEX8(0xFFU, result);
}

void test_out_of_range_access_is_rejected() {
    removeTestFiles();

    eurorack::storage::FileStorage storage(testPath().string(), 8U);

    std::uint8_t value = 0U;
    TEST_ASSERT_EQUAL_INT(static_cast<int>(eurorack::io::IoResult::InvalidArgument),
                          static_cast<int>(storage.read(8U, &value, 1U)));
}

} // namespace

extern "C" {

void setUp() {}

void tearDown() {
    removeTestFiles();
}

} // extern "C"

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_missing_file_starts_erased);
    RUN_TEST(test_commit_and_reopen_preserves_bytes);
    RUN_TEST(test_erase_marks_storage_dirty);
    RUN_TEST(test_out_of_range_access_is_rejected);

    return UNITY_END();
}
