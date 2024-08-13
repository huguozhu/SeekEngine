#include "util.h"

SEEK_NAMESPACE_BEGIN

std::string string_trim(std::string const& str)
{
    if (str.empty()) return "";
    int off1 = 0;
    int off2 = int(str.size()) - 1;
    while (off1 < int(str.size()) && str[off1] == ' ') off1++;
    while (off2 >= 0 && (str[off2] == ' ' || str[off2] == '\r' || str[off2] == '\n')) off2--;
    if (off1 >= int(str.size()) || off2 < 0)
        return "";
    return str.substr(off1, size_t(off2 - off1 + 1));
}

std::vector<std::string> string_split(std::string const& str, char delimiter)
{
    std::vector<std::string> result;
    size_t off = 0;
    while (1)
    {
        size_t delim = str.find_first_of(delimiter, off);
        if (delim == std::string::npos) break;

        result.push_back(str.substr(off, delim - off));
        off = delim + 1;
    }
    result.push_back(str.substr(off, str.size() - off));
    return result;
}

void string_replace(std::string & str, std::string const& from, std::string const& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#if defined(SEEK_PLATFORM_WINDOWS)
#include "windows.h"
std::wstring utf8_to_wchar(const char* src)
{
    std::wstring result;

    int size_needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, NULL, 0);
    if (size_needed <= 0)
        return result;

    result.resize(size_needed);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &result[0], size_needed);
    return result;
}
#endif

SEEK_NAMESPACE_END
