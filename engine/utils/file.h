#include "util.h"
//#include "buildtime_client_config.h"

#if defined(BUILD_FOR_GOOD) || defined(ANDROID_BUILD_FOR_GOOD)
#define USE_GD_FILE
#endif

#ifdef USE_GD_FILE
#include "GDFile.h"
#endif

#define SEEK_MACRO_FILE_UID 74     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

inline FILE* dvf_fopen(const char* utf8FilePath, const char* mode)
{
    if (!utf8FilePath || !mode)
        return nullptr;
#ifdef USE_GD_FILE
    return GDDynamic_fopen(utf8FilePath, mode);
#else
#   ifdef SEEK_PLATFORM_WINDOWS
    std::wstring utf16FilePath = utf8_to_wchar(utf8FilePath);
    std::wstring utf16mode = utf8_to_wchar(mode);
    return _wfopen(utf16FilePath.c_str(), utf16mode.c_str());
#   else
    return fopen(utf8FilePath, mode);
#   endif
#endif
}

#ifdef USE_GD_FILE
    #define dvf_fclose      GDDynamic_fclose
    #define dvf_fflush      GDDynamic_fflush
    #define dvf_ftell       GDDynamic_ftell
    #define dvf_fread       GDDynamic_fread
    #define dvf_fseek       GDDynamic_fseek
#else
    #define dvf_fclose      fclose
    #define dvf_fflush      fflush
    #define dvf_ftell       ftell
    #define dvf_fread       fread
    #define dvf_fseek       fseek
#endif

inline SResult read_file_content(const char* filePath, const char* mode, std::vector<uint8_t>& content)
{
    FILE* _file = dvf_fopen(filePath, mode);
    if (!_file)
#ifdef USE_GD_FILE
        return ERR_FILE_NOT_FOUND; // Use line numbers to judge which fopen run
#else
        return ERR_FILE_NOT_FOUND;
#endif

    dvf_fseek(_file, 0, SEEK_END);
    auto _size = dvf_ftell(_file);
    dvf_fseek(_file, 0, SEEK_SET);
    content.resize(_size);
    dvf_fread((void*)content.data(), 1, _size, _file);
    dvf_fclose(_file);
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
