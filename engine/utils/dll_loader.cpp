#include "utils/dll_loader.h"

#if defined(SEEK_PLATFORM_WINDOWS)
#include <Windows.h>
#else
#endif

SEEK_NAMESPACE_BEGIN

bool DllLoader::Load()
{
    if (module) return true;
#if defined(SEEK_PLATFORM_WINDOWS)
    module = (void*)LoadLibraryA(dllname.c_str());
    return module != nullptr;
#else
    return false;
#endif
}

void DllLoader::Unload()
{
    if (module)
    {
#if defined(SEEK_PLATFORM_WINDOWS)
        FreeLibrary((HMODULE)module);
#else
#endif
    }
}

void* DllLoader::FindSymbol(const std::string& funcname)
{
    if (!module) return nullptr;
#if defined(SEEK_PLATFORM_WINDOWS)
    return (void*)::GetProcAddress((HMODULE)module, funcname.c_str());
#else
    return nullptr;
#endif
}

SEEK_NAMESPACE_END
