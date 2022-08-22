#ifndef WFREST_STRUTIL_H_
#define WFREST_STRUTIL_H_

#include "workflow/StringUtil.h"
#include <string>
#include "wfrest/StringPiece.h"

namespace wfrest
{

extern const std::string string_not_found;

class StrUtil : public StringUtil
{
public:
    static StringPiece trim_pairs(const StringPiece &str, const char *pairs = k_pairs_.c_str());

    static StringPiece ltrim(const StringPiece &str);

    static StringPiece rtrim(const StringPiece &str);

    static StringPiece trim(const StringPiece &str);

    static char* strsep(char** stringp, const char* delim);

    template<class OutputStringType>
    static std::vector<OutputStringType> split_piece(const StringPiece &str, char sep);
    
    //static std::vector<std::string> split(const std::string& s, const std::string& d);

    static std::string& ltrim(std::string& s);

    static std::string& rtrim(std::string& s);

    static std::string& trim(std::string& s);

    static bool startsWith(const std::string& str, const std::string& prefix);

    static bool endsWith(const std::string& str, const std::string& suffix);

    static std::string::size_type indexOf(const std::string& str, const std::string& substr);

    static std::string toUpper(const std::string& str);

    static std::string toLower(const std::string& str);

    static std::string format(const char* fmt, ...);
private:
    static const std::string k_pairs_;
};


template<class OutputStringType>
std::vector<OutputStringType> StrUtil::split_piece(const StringPiece &str, char sep)
{
    std::vector<OutputStringType> res;
    if (str.empty())
        return res;

    const char *p = str.begin();
    const char *cursor = p;

    while (p != str.end())
    {
        if (*p == sep)
        {
            res.emplace_back(OutputStringType(cursor, p - cursor));
            cursor = p + 1;
        }
        ++p;
    }
    res.emplace_back(OutputStringType(cursor, str.end() - cursor));
    return res;
}

// std::map<std::string, std::string, MapStringCaseLess>
class MapStringCaseLess {
public:
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

}  // namespace wfrest

#endif // WFREST_STRUTIL_H_
