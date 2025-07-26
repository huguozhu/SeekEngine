#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_texture.h"

SEEK_NAMESPACE_BEGIN


RHIMeshPtr    D3D12RHIContext::CreateMesh()
{
    return nullptr;
}
RHIShaderPtr  D3D12RHIContext::CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
{
    return nullptr;
}
RHITexturePtr D3D12RHIContext::CreateTexture2D(ID3D12ResourcePtr const& tex)
{
    return MakeSharedPtr<D3D12Texture2D>(m_pContext, tex);
}
RHITexturePtr D3D12RHIContext::CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12Texture2DPtr tex = MakeSharedPtr<D3D12Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D12RHIContext::CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12Texture3DPtr tex = MakeSharedPtr<D3D12Texture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D12RHIContext::CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D12TextureCubePtr tex = MakeSharedPtr<D3D12TextureCube>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHIRenderBufferPtr D3D12RHIContext::CreateConstantBuffer(uint32_t size, ResourceFlags flags)
{ 
    return nullptr; 
}
RHIRenderBufferPtr D3D12RHIContext::CreateStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData)
{ 
    return nullptr; 
}
RHIRenderBufferPtr D3D12RHIContext::CreateRWStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData)
{ 
    return nullptr; 
}
RHIRenderBufferPtr D3D12RHIContext::CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{ 
    return nullptr;
}
RHIRenderBufferPtr D3D12RHIContext::CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    return nullptr; 
}
RHIRenderBufferPtr D3D12RHIContext::CreateVertexBuffer(uint32_t size, RHIRenderBufferData* pData)
{ 
    return nullptr;
}
RHIRenderBufferPtr D3D12RHIContext::CreateIndexBuffer(uint32_t size, RHIRenderBufferData* pData)
{ 
    return nullptr;
}
















RHIFencePtr D3D12RHIContext::CreateFence()
{
    return MakeSharedPtr<D3D12Fence>(m_pContext);
}

SEEK_NAMESPACE_END