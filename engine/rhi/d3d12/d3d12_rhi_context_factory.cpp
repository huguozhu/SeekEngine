#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_texture.h"

SEEK_NAMESPACE_BEGIN


RHIMeshPtr    D3D12Context::CreateMesh()
{
    return nullptr;
}
RHIShaderPtr  D3D12Context::CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
{
    return nullptr;
}
RHITexturePtr D3D12Context::CreateTexture2D(ID3D12ResourcePtr const& tex)
{
    return MakeSharedPtr<D3D12Texture2D>(m_pContext, tex);
}
RHITexturePtr D3D12Context::CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12Texture2DPtr tex = MakeSharedPtr<D3D12Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D12Context::CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12Texture3DPtr tex = MakeSharedPtr<D3D12Texture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D12Context::CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12TextureCubePtr tex = MakeSharedPtr<D3D12TextureCube>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHIGpuBufferPtr D3D12Context::CreateGpuBuffer(uint32_t size, ResourceFlags usage)
{
    return nullptr;
}
RHIGpuBufferPtr D3D12Context::CreateConstantBuffer(uint32_t size, ResourceFlags flags)
{ 
    return nullptr; 
}
RHIGpuBufferPtr D3D12Context::CreateStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIGpuBufferData* pData)
{ 
    return nullptr; 
}
RHIGpuBufferPtr D3D12Context::CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIGpuBufferData* pData)
{ 
    return nullptr;
}
RHIGpuBufferPtr D3D12Context::CreateVertexBuffer(uint32_t size, RHIGpuBufferData* pData)
{ 
    return nullptr;
}
RHIGpuBufferPtr D3D12Context::CreateIndexBuffer(uint32_t size, RHIGpuBufferData* pData)
{ 
    return nullptr;
}
















RHIFencePtr D3D12Context::CreateFence()
{
    return MakeSharedPtr<D3D12Fence>(m_pContext);
}

SEEK_NAMESPACE_END