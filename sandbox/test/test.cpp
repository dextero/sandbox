#include "test.h"

std::unique_ptr<std::map<std::string, TestSuite>> all_tests;
LastTestError last_test_error;

#define INDENT "  "
#define INDENT2 INDENT INDENT

using namespace sb;

int main()
{
    using Color = Logger<FileOutput>::Color;

    size_t total_tests = 0;
    std::vector<std::string> failed_test_names;

    for (const auto& name_suite_pair: *all_tests) {
        const std::string& suite_name = name_suite_pair.first;
        const TestSuite& suite = name_suite_pair.second;
        size_t failed_tests = 0;

        total_tests += suite.size();

        gLog.printf(Color::Yellow, "%s: start\n", suite_name.c_str());

        for (const auto& name_test_pair: suite) {
            const std::string& test_name = name_test_pair.first;
            int (*test)(void) = name_test_pair.second;

            gLog.printf(Color::White, INDENT "%-40s\r", test_name.c_str());

            if (test() == TEST_SUCCESS) {
                gLog.printf(Color::Green, INDENT "%-40s OK\n", test_name.c_str());
                continue;
            }

            ++failed_tests;
            failed_test_names.push_back(suite_name + "/" + test_name);

            gLog.printf(Color::Red, INDENT "%-40s FAIL\n", test_name.c_str());
            gLog.printf(Color::Red, INDENT2 "%s\n",
                        last_test_error.message.c_str());
            gLog.printf(Color::White, INDENT2 "at %s:%d\n",
                        last_test_error.file, last_test_error.line);

            last_test_error = { "", "", 0 };
        }

        gLog.printf(Color::Yellow, "%s: ", suite_name.c_str());
        Color color = (failed_tests == 0) ? Color::Green
                                                  : Color::Red;
        gLog.printf(color, "%u/%u\n", (unsigned)(suite.size() - failed_tests),
                    (unsigned)suite.size());
    }

    gLog.printf(Color::Yellow, "\n-----\nTOTAL: %u tests, ",
                (unsigned)total_tests);

    if (failed_test_names.empty()) {
        gLog.printf(Color::Green, "all passed\n");
    } else {
        gLog.printf(Color::Green, "%u passed",
                    (unsigned)(total_tests - failed_test_names.size()));
        gLog.printf(Color::White, ", ");
        gLog.printf(Color::Red, "%u failed:\n",
                    (unsigned)failed_test_names.size());

        for (const std::string& test: failed_test_names) {
            gLog.printf(Color::White, INDENT "%s\n", test.c_str());
        }
    }

    return 0;
}

