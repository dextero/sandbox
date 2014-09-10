#pragma once

#include <map>
#include <memory>

#include "utils/logger.h"
#include "utils/stringUtils.h"

struct LastTestError
{
    std::string message;
    const char* file;
    int line;
};

typedef std::map<std::string, int(*)(void)> TestSuite;

extern std::unique_ptr<std::map<std::string, TestSuite>> all_tests;
extern LastTestError last_test_error;

#define TEST_CASE(suite_name, test_name) \
    static int test_ ## suite_name ## _ ## test_name (); \
    static void test_ ## suite_name ## _ ## test_name ## _init() __attribute__((constructor)); \
    static void test_ ## suite_name ## _ ## test_name ## _init() \
    { \
        if (!all_tests) { \
            all_tests.reset(new std::map<std::string, TestSuite>()); \
        } \
        if ((*all_tests)[#suite_name].find(#test_name) != (*all_tests)[#suite_name].end()) { \
            sbFail("duplicate test name: %s/%s", #suite_name, #test_name); \
        } \
        (*all_tests)[#suite_name][#test_name] = test_ ## suite_name ## _ ## test_name; \
    } \
    static int test_ ## suite_name ## _ ## test_name()

#define TEST_SUCCESS 0
#define TEST_FAIL 1

#define FAIL(message, ...) \
    do { \
        last_test_error = { \
            ::sb::utils::format("assertion failed: " message, __VA_ARGS__), \
            __FILE__, __LINE__ \
        }; \
        return TEST_FAIL; \
    } while (0)

#define _ASSERT(expected, op, actual) \
    if (!((expected) op (actual))) { \
        FAIL("{0} " #op " {1}", (expected), (actual)); \
    }

#define ASSERT_EQUAL(expected, actual)         _ASSERT((expected), ==, (actual))
#define ASSERT_NOT_EQUAL(expected, actual)     _ASSERT((expected), !=, (actual))
#define ASSERT_LESS(expected, actual)          _ASSERT((expected), <, (actual))
#define ASSERT_LESS_OR_EQUAL(expected, actual) _ASSERT((expected), <=, (actual))
#define ASSERT_MORE(expected, actual)          _ASSERT((expected), >, (actual))
#define ASSERT_MORE_OR_EQUAL(expected, actual) _ASSERT((expected), >=, (actual))
#define ASSERT_NULL(actual)                    ASSERT_EQUAL(NULL, (actual))
#define ASSERT_NOT_NULL(actual)                ASSERT_NOT_EQUAL(NULL, (actual))
#define ASSERT_TRUE(actual)                    ASSERT_EQUAL(true, (actual))
#define ASSERT_FALSE(actual)                   ASSERT_EQUAL(false, (actual))

