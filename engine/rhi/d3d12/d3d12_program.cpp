#include "rhi/d3d12/d3d12_program.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_context.h"

SEEK_NAMESPACE_BEGIN

SResult D3D12Program::Active()
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());

    for (size_t i = 0; i < m_vShaders.size(); i++)
    {
        if (m_vShaders[i])
        {
            D3D12Shader& shader = static_cast<D3D12Shader&>(*m_vShaders[i]);
            SEEK_RETIF_FAIL(shader.Active());
        }
        else
        {

        }
    }
    return S_Success;
}

void D3D12Program::Deactive()
{
    for (size_t i = 0; i < m_vShaders.size(); i++)
    {
        if (m_vShaders[i])
        {
            D3D12Shader& shader = static_cast<D3D12Shader&>(*m_vShaders[i]);
            shader.Deactive();
        }
    }
}


SEEK_NAMESPACE_END
