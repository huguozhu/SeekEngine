#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_context.h"
#include "kernel/context.h"

#pragma comment(lib, "dxcompiler.lib")

SEEK_NAMESPACE_BEGIN

SResult D3D12Shader::Active()
{
    if (!m_bCompileReady)
    {
        SEEK_RETIF_FAIL(this->OnCompile());
        m_bCompileReady = true;
    }
    return S_Success;
}
void D3D12Shader::Deactive()
{
}
SResult D3D12Shader::OnCompile()
{
    //IDxcCompilerPtr pCompiler;
    //DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
    D3D12_SHADER_BYTECODE shaderByteCode;
    
    //pCompiler->Compile()

    return S_Success;
}
void D3D12Shader::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const
{
}
void D3D12Shader::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const
{

}
SEEK_NAMESPACE_END
