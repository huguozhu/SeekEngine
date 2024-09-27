#include "rhi/base/rhi_program.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 41     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SResult RHIProgram::SetShader(RHIShader* shader)
{
    if (!shader) return ERR_INVALID_ARG;
    m_vShaders[int(shader->Type())] = shader;

    char* begin = (char*)this;
    m_Hash = HashRange(begin, begin+sizeof(RHIProgram));
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
