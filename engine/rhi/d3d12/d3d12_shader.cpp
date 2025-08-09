#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

SResult D3D12Shader::Active()
{
    return S_Success;
}
void D3D12Shader::Deactive()
{
}
SResult D3D12Shader::OnCompile()
{
    return S_Success;
}


SEEK_NAMESPACE_END
