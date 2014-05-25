#include "utils/stringUtils.h"
#include "utils/logger.h"

#include <fstream>

namespace sb
{
    namespace utils
    {
        std::string toString(const std::wstring& wstr)
        {
            return std::string(wstr.begin(), wstr.end());
        }

        std::wstring toWString(const std::string& str)
        {
            return std::wstring(str.begin(), str.end());
        }

        std::vector<std::string> split(const std::string& str, char c)
        {
            std::vector<std::string> ret;

            size_t prev = 0, at = (size_t)-1;
            while ((at = str.find(c, at + 1)) != std::string::npos)
            {
                if (at != prev + 1) {
                    ret.push_back(str.substr(prev, at - prev));
                }

                prev = at;
            }
            ret.push_back(str.substr(prev));

            return ret;
        }

        std::vector<std::string>
        split(const std::string& str,
              const std::function<bool(char)>& isSeparator)
        {
            std::vector<std::string> ret;

            size_t prev = 0;
            for (size_t at = 0; at < str.size(); ++at) {
                if (!isSeparator(str[at])) {
                    continue;
                } else if (prev == at) {
                    ++prev;
                    continue;
                }

                ret.push_back(str.substr(prev, at - prev));
                prev = at + 1;
            }

            if (prev != str.size()) {
                ret.push_back(str.substr(prev));
            }

            return ret;
        }

        std::string readFile(const std::string& path)
        {
            std::ifstream file(path);

            if (!file.is_open()) {
                return "";
            }

            file.get();
            if (!file) {
                gLog.err("invalid file: %s, maybe it is a directory?\n",
                           path.c_str());
                return "";
            }

            file.unget();

            file.seekg(0, std::ios::end);
            size_t filesize = file.tellg();

            gLog.debug("reading %lu bytes from file %s\n",
                       filesize, path.c_str());

            std::string contents;
            contents.resize(filesize);
            file.seekg(0, std::ios::beg);

            file.read(&contents[0], contents.size());
            return contents;
        }
    } // namespace utils
} // namespace sb
