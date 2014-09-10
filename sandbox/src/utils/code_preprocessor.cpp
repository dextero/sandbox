#include "utils/code_preprocessor.h"

namespace sb {
namespace {

bool isIncludeLine(const std::string& line)
{
    size_t firstPrintableAt = line.find_first_not_of(" \t\v\f");
    if (firstPrintableAt == std::string::npos) {
        return false;
    }

    return (line.substr(firstPrintableAt, 8) == "#include");
}

std::string extractIncludeFilename(const std::string &line)
{
    std::string stripped = utils::strip(line);
    if (stripped.find("#include") != 0) {
        return "";
    }

    return utils::strip(stripped.substr(8), " \t\"<>");
}

} // namespace

Node loadPreprocessed(const std::string &filename,
                      std::set<std::string> &includedFiles)
{
    Node root;

    //gLog.debug("subcall: %s", filename.c_str());
    if (includedFiles.count(filename) > 0) {
        sbFail("shader file \"%s\" included more than once", filename.c_str());
    }
    includedFiles.insert(filename);

    root.text = filename;
    std::string code = utils::readFile(filename);
    std::vector<std::string> lines = utils::split(code, "\n");

    for (const std::string &line: lines) {
        if (!isIncludeLine(line)) {
            //gLog.debug("line: %s", line.c_str());
            root.subNodes.emplace_back(line);
            continue;
        }

        std::string includeFilename = extractIncludeFilename(line);
        //gLog.debug("include: %s", includeFilename.c_str());
        root.subNodes.push_back(loadPreprocessed(includeFilename, includedFiles));
    }

    return root;
}

} // namespace sb

