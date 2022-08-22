#include "wfrest/StrUtil.h"
#include <cstdarg>
#include <algorithm>
#ifndef OS_WINDOWS
#include <cstring>
#endif // OS_WINDOWS


using namespace wfrest;

const std::string wfrest::string_not_found = "";
const std::string StrUtil::k_pairs_ = R"({}[]()<>""''``)";

StringPiece StrUtil::trim_pairs(const StringPiece &str, const char *pairs)
{
    const char *lhs = str.begin();
    const char *rhs = str.begin() + str.size() - 1;
    const char *p = pairs;
    bool is_pair = false;
    while (*p != '\0' && *(p + 1) != '\0')
    {
        if (*lhs == *p && *rhs == *(p + 1))
        {
            is_pair = true;
            break;
        }
        p += 2;
    }
    return is_pair ? StringPiece(str.begin() + 1, str.size() - 2) : str;
}

StringPiece StrUtil::ltrim(const StringPiece &str)
{
    const char *lhs = str.begin();
    while (lhs != str.end() && std::isspace(*lhs)) lhs++;
    if (lhs == str.end()) return {};
    StringPiece res(str);
    res.remove_prefix(lhs - str.begin());
    return res;
}

StringPiece StrUtil::rtrim(const StringPiece &str)
{
    if (str.empty()) return str;
    const char *rhs = str.end() - 1;
    while (rhs != str.begin() && std::isspace(*rhs)) rhs--;
    if (rhs == str.begin() && std::isspace(*rhs)) return {};
    StringPiece res(str.begin(), rhs - str.begin() + 1);
    return res;
}

StringPiece StrUtil::trim(const StringPiece &str)
{
    return ltrim(rtrim(str));
}
#ifdef OS_WINDOWS
/*
* Get next token from string *stringp, where tokens are possibly-empty
* strings separated by characters from delim.
*
* Writes NULs into the string at *stringp to end tokens.
* delim need not remain constant from call to call.
* On return, *stringp points past the last NUL written (if there might
* be further tokens), or is NULL (if there are definitely no moretokens).
*
* If *stringp is NULL, strsep returns NULL.
*/
char* StrUtil::strsep(char** stringp, const char* delim)
{
    char* s;
    const char* spanp;
    int c, sc;
    char* tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}
#endif // OS_WINDOWS

/*
std::vector<std::string> StrUtil::split(const std::string& s, const std::string& d)
{
    std::vector<std::string> v;
    char* str = new char[s.size() + 1];
    strcpy(str, s.c_str());
    while (char* t = strsep(&str, d.c_str()))
        v.push_back(t);
    delete[] str;
    return v;
}
*/

std::string& StrUtil::ltrim(std::string& s)
{
    if (s.empty()) return s;
    std::string::const_iterator iter = s.begin();
    while (iter != s.end() && isspace(*iter++));
    s.erase(s.begin(), --iter);
    return s;
}


std::string& StrUtil::rtrim(std::string& s)
{
    if (s.empty()) return s;
    std::string::const_iterator iter = s.end();
    while (iter != s.begin() && isspace(*--iter));
    s.erase(++iter, s.end());
    return s;
}


std::string& StrUtil::trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
    return s;
}


bool StrUtil::startsWith(const std::string& str, const std::string& prefix)
{
    return prefix.size() <= str.size() &&
        std::equal(prefix.cbegin(), prefix.cend(), str.cbegin());
}


bool StrUtil::endsWith(const std::string& str, const std::string& suffix)
{
    return suffix.size() <= str.size() &&
        std::equal(suffix.crbegin(), suffix.crend(), str.crbegin());
}


std::string::size_type StrUtil::indexOf(const std::string& str, const std::string& substr)
{
    return str.find(substr);
}


std::string StrUtil::toUpper(const std::string& str)
{
    std::string upper(str.size(), '\0');
    std::transform(str.cbegin(), str.cend(), upper.begin(), ::toupper);
    return upper;
}


std::string StrUtil::toLower(const std::string& str)
{
    std::string lower(str.size(), '\0');
    std::transform(str.cbegin(), str.cend(), lower.begin(), ::tolower);
    return lower;
}


std::string StrUtil::format(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);
    std::string buf(len + 1, '\0');
    va_start(ap, fmt);
    vsnprintf(&buf[0], buf.size(), fmt, ap);
    va_end(ap);
    buf.pop_back();
    return buf;
}