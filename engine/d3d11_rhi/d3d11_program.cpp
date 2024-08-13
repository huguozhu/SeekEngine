#include "rendering_d3d11/d3d11_program.h"
#include "rendering_d3d11/d3d11_shader.h"
#include "rendering_d3d11/d3d11_render_context.h"

#define DVF_MACRO_FILE_UID 7     // this code is auto generated, don't touch it!!!

DVF_NAMESPACE_BEGIN

DVFResult D3D11Program::Active()
{
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());

    for (size_t i = 0; i < m_vShaders.size(); i++)
    {
        if (m_vShaders[i])
        {
            D3D11Shader& shader = static_cast<D3D11Shader&>(*m_vShaders[i]);
            DVF_RETIF_FAIL(shader.Active());
        }
        else
        {
            // TODO: set only when needed
            rc.SetD3DShader((ShaderType)i, nullptr);
        }
    }
    return DVF_Success;
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

DVF_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
