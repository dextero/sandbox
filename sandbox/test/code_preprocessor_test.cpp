#include "test.h"
#include "utils/code_preprocessor.cpp"

#include <fstream>
#include <unistd.h>

using namespace sb;

namespace {

struct TempFile
{
    const std::string filename;

    static std::string make_temp(const std::string& content)
    {
        char name[] = "/tmp/tmpXXXXXX";
        int fd = mkstemp(name);
        (void)fd;
        std::ofstream(name) << content;
        return name;
    }

    TempFile(const std::string &content):
        filename(make_temp(content))
    {
    }

    ~TempFile()
    {
        if (!filename.empty()) {
            unlink(filename.c_str());
        }
    }
};

} // namespace

TEST_CASE(code_preprocessor, iterator_empty)
{
    Node root { "someTempFile" };
    root.subNodes = { { "" } };

    NodeIterator<const Node> it { root };
    NodeIterator<const Node> end { root, nullptr };

    ASSERT_TRUE(it.valid());
    ASSERT_TRUE(it->isLine());
    ASSERT_EQUAL("", it->getLine());
    ASSERT_NOT_EQUAL(end, it);

    ASSERT_EQUAL(&it, &++it);
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(end, it);

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, parse_empty)
{
    TempFile file("");
    PreprocessedCode code(file.filename);

    ASSERT_EQUAL(1, code.getNumLines());
    ASSERT_EQUAL("", code.getCode());

    for (const auto& node: code) {
        ASSERT_EQUAL("", node.getLine());
    }

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, parse_single_line)
{
    TempFile file("test");
    PreprocessedCode code(file.filename);

    ASSERT_EQUAL(1, code.getNumLines());
    ASSERT_EQUAL("test", code.getCode());

    auto it = code.begin();
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("test", it->getLine());
    ++it;
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(code.end(), it);

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, parse_multiple_lines)
{
    TempFile file("first\nsecond\nthird");
    PreprocessedCode code(file.filename);

    ASSERT_EQUAL(3, code.getNumLines());
    ASSERT_EQUAL("first\nsecond\nthird", code.getCode());

    auto it = code.begin();
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("first", it->getLine());
    ++it;
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("second", it->getLine());
    ++it;
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("third", it->getLine());
    ++it;
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(code.end(), it);

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, include_only)
{
    TempFile included_file("included");
    TempFile root_file("#include \"" + included_file.filename + "\"");
    PreprocessedCode code(root_file.filename);

    ASSERT_EQUAL(1, code.getNumLines());
    ASSERT_EQUAL("included", code.getCode());

    auto it = code.begin();
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("included", it->getLine());

    std::vector<BacktraceNode> backtrace = it.getBacktrace();
    ASSERT_EQUAL(2, backtrace.size());
    ASSERT_EQUAL(included_file.filename, backtrace[0].file);
    ASSERT_EQUAL(1, backtrace[0].line);
    ASSERT_EQUAL(root_file.filename, backtrace[1].file);
    ASSERT_EQUAL(1, backtrace[1].line);

    ++it;
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(code.end(), it);

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, include_and_text)
{
    TempFile included_file("second");
    TempFile root_file("first\n#include \"" + included_file.filename + "\"\nthird");
    PreprocessedCode code(root_file.filename);

    ASSERT_EQUAL(3, code.getNumLines());
    ASSERT_EQUAL("first\nsecond\nthird", code.getCode());

    auto it = code.begin();
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("first", it->getLine());

    std::vector<BacktraceNode> backtrace = it.getBacktrace();
    ASSERT_EQUAL(1, backtrace.size());
    ASSERT_EQUAL(root_file.filename, backtrace[0].file);
    ASSERT_EQUAL(1, backtrace[0].line);

    ++it;
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("second", it->getLine());

    backtrace = it.getBacktrace();
    ASSERT_EQUAL(2, backtrace.size());
    ASSERT_EQUAL(included_file.filename, backtrace[0].file);
    ASSERT_EQUAL(1, backtrace[0].line);
    ASSERT_EQUAL(root_file.filename, backtrace[1].file);
    ASSERT_EQUAL(2, backtrace[1].line);

    ++it;
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("third", it->getLine());

    backtrace = it.getBacktrace();
    ASSERT_EQUAL(1, backtrace.size());
    ASSERT_EQUAL(root_file.filename, backtrace[0].file);
    ASSERT_EQUAL(3, backtrace[0].line);

    ++it;
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(code.end(), it);

    return TEST_SUCCESS;
}

TEST_CASE(code_preprocessor, nested_include)
{
    TempFile included2_file("included");
    TempFile included_file("#include \"" + included2_file.filename + "\"");
    TempFile root_file("#include \"" + included_file.filename + "\"");
    PreprocessedCode code(root_file.filename);

    ASSERT_EQUAL(1, code.getNumLines());
    ASSERT_EQUAL("included", code.getCode());

    auto it = code.begin();
    ASSERT_TRUE(it.valid());
    ASSERT_EQUAL("included", it->getLine());

    std::vector<BacktraceNode> backtrace = it.getBacktrace();
    ASSERT_EQUAL(3, backtrace.size());
    ASSERT_EQUAL(included2_file.filename, backtrace[0].file);
    ASSERT_EQUAL(1, backtrace[0].line);
    ASSERT_EQUAL(included_file.filename, backtrace[1].file);
    ASSERT_EQUAL(1, backtrace[1].line);
    ASSERT_EQUAL(root_file.filename, backtrace[2].file);
    ASSERT_EQUAL(1, backtrace[2].line);

    ++it;
    ASSERT_FALSE(it.valid());
    ASSERT_EQUAL(code.end(), it);

    return TEST_SUCCESS;
}
