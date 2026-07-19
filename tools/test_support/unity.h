#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace unity_detail {

inline int failures = 0;
inline int tests = 0;
inline const char* current_test = "";

inline void fail(
    const char* expression,
    const char* file,
    int line) {
    ++failures;
    std::fprintf(
        stderr,
        "[FAIL] %s: %s at %s:%d\n",
        current_test,
        expression,
        file,
        line);
}

template <typename Expected, typename Actual>
inline void assert_equal(
    Expected expected,
    Actual actual,
    const char* expression,
    const char* file,
    int line) {
    if (expected != actual) {
        fail(expression, file, line);
    }
}

inline int begin() {
    failures = 0;
    tests = 0;
    return 0;
}

inline int end() {
    std::fprintf(
        stdout,
        "[UNITY] %d tests, %d failures\n",
        tests,
        failures);
    return failures;
}

}  // namespace unity_detail

#define UNITY_BEGIN() unity_detail::begin()
#define UNITY_END() unity_detail::end()

#define RUN_TEST(function)                                              \
    do {                                                                \
        ++unity_detail::tests;                                          \
        unity_detail::current_test = #function;                         \
        setUp();                                                        \
        function();                                                     \
        tearDown();                                                     \
    } while (false)

#define TEST_ASSERT_TRUE(condition)                                     \
    do {                                                                \
        if (!(condition)) {                                             \
            unity_detail::fail(#condition, __FILE__, __LINE__);         \
        }                                                               \
    } while (false)

#define TEST_ASSERT_FALSE(condition) TEST_ASSERT_TRUE(!(condition))

#define TEST_ASSERT_EQUAL_INT(expected, actual)                         \
    unity_detail::assert_equal(                                         \
        static_cast<long long>(expected),                               \
        static_cast<long long>(actual),                                 \
        #actual, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_INT16(expected, actual)                       \
    TEST_ASSERT_EQUAL_INT(expected, actual)

#define TEST_ASSERT_EQUAL_INT32(expected, actual)                       \
    TEST_ASSERT_EQUAL_INT(expected, actual)

#define TEST_ASSERT_EQUAL_UINT8(expected, actual)                       \
    unity_detail::assert_equal(                                         \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint8_t>(expected)),                       \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint8_t>(actual)),                         \
        #actual, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_UINT16(expected, actual)                      \
    unity_detail::assert_equal(                                         \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint16_t>(expected)),                      \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint16_t>(actual)),                        \
        #actual, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_UINT32(expected, actual)                      \
    unity_detail::assert_equal(                                         \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint32_t>(expected)),                      \
        static_cast<unsigned long long>(                                \
            static_cast<std::uint32_t>(actual)),                        \
        #actual, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_UINT64(expected, actual)                      \
    unity_detail::assert_equal(                                         \
        static_cast<unsigned long long>(expected),                      \
        static_cast<unsigned long long>(actual),                        \
        #actual, __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_HEX8(expected, actual)                        \
    TEST_ASSERT_EQUAL_UINT8(expected, actual)

#define TEST_ASSERT_EQUAL_HEX16(expected, actual)                       \
    TEST_ASSERT_EQUAL_UINT16(expected, actual)

#define TEST_ASSERT_EQUAL_HEX32(expected, actual)                       \
    TEST_ASSERT_EQUAL_UINT32(expected, actual)

#define TEST_ASSERT_NOT_EQUAL(expected, actual)                         \
    do {                                                                \
        if ((expected) == (actual)) {                                   \
            unity_detail::fail(#actual, __FILE__, __LINE__);            \
        }                                                               \
    } while (false)

#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)               \
    do {                                                                \
        const double unity_difference =                                 \
            std::fabs(                                                  \
                static_cast<double>(expected)                           \
                - static_cast<double>(actual));                         \
        if (unity_difference > static_cast<double>(delta)) {            \
            unity_detail::fail(#actual, __FILE__, __LINE__);            \
        }                                                               \
    } while (false)

#define TEST_ASSERT_UINT32_WITHIN(delta, expected, actual)              \
    do {                                                                \
        const std::uint32_t unity_expected =                            \
            static_cast<std::uint32_t>(expected);                       \
        const std::uint32_t unity_actual =                              \
            static_cast<std::uint32_t>(actual);                         \
        const std::uint32_t unity_difference =                          \
            unity_expected > unity_actual                              \
            ? unity_expected - unity_actual                            \
            : unity_actual - unity_expected;                           \
        if (unity_difference > static_cast<std::uint32_t>(delta)) {     \
            unity_detail::fail(#actual, __FILE__, __LINE__);            \
        }                                                               \
    } while (false)

#define TEST_ASSERT_EQUAL_STRING(expected, actual)                      \
    do {                                                                \
        const char* unity_expected = (expected);                        \
        const char* unity_actual = (actual);                            \
        if (unity_expected == nullptr                                  \
            || unity_actual == nullptr                                 \
            || std::strcmp(unity_expected, unity_actual) != 0) {        \
            unity_detail::fail(#actual, __FILE__, __LINE__);            \
        }                                                               \
    } while (false)

#define TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, count)          \
    do {                                                                \
        const auto* unity_expected =                                    \
            reinterpret_cast<const std::uint8_t*>(expected);            \
        const auto* unity_actual =                                      \
            reinterpret_cast<const std::uint8_t*>(actual);              \
        if (std::memcmp(                                                 \
                unity_expected,                                         \
                unity_actual,                                           \
                static_cast<std::size_t>(count)) != 0) {                \
            unity_detail::fail(#actual, __FILE__, __LINE__);            \
        }                                                               \
    } while (false)
