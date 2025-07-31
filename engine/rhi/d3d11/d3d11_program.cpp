#include "rhi/d3d11/d3d11_program.h"
#include "rhi/d3d11/d3d11_shader.h"
#include "rhi/d3d11/d3d11_context.h"

#define SEEK_MACRO_FILE_UID 7     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SResult D3D11Program::Active()
{
    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());

    for (size_t i = 0; i < m_vShaders.size(); i++)
    {
        if (m_vShaders[i])
        {
            D3D11Shader& shader = static_cast<D3D11Shader&>(*m_vShaders[i]);
            SEEK_RETIF_FAIL(shader.Active());
        }
        else
        {
            // TODO: set only when needed
            rc.SetD3DShader((ShaderType)i, nullptr);
        }
    }
    return S_Success;
}

void D3D11Program::Deactive()
{
    for (size_t i = 0; i < m_vShaders.size(); i++)
    {
        if (m_vShaders[i])
        {
            D3D11Shader& shader = static_cast<D3D11Shader&>(*m_vShaders[i]);
            shader.Deactive();
        }
    }
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
