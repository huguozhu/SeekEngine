#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_texture.h"
#include "rhi/d3d12/d3d12_gpu_buffer.h"

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
RHIGpuBufferPtr D3D12Context::CreateGpuBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_stride, RHIGpuBufferData* pData)
{
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D12GpuBuffer>(m_pContext, size, flags, structure_stride);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D12Context::CreateConstantBuffer(uint32_t size, ResourceFlags flags, RHIGpuBufferData* pData)
{ 
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D12GpuBuffer>(m_pContext, size, flags);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D12Context::CreateVertexBuffer(uint32_t size, RHIGpuBufferData* pData)
{ 
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D12GpuBuffer>(m_pContext, size, RESOURCE_FLAG_CPU_WRITE| RESOURCE_FLAG_GPU_READ);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D12Context::CreateIndexBuffer(uint32_t size, RHIGpuBufferData* pData)
{ 
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D12GpuBuffer>(m_pContext, size, RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
    buf->Create(pData);
    return buf;
}

RHIShaderResourceViewPtr D3D12Context::CreateBufferSrv(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
    return MakeSharedPtr<D3D12BufferSrv>(m_pContext, buffer, format, first_elem, num_elems);
}
RHIUnorderedAccessViewPtr D3D12Context::CreateBufferUav(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{ 
    return MakeSharedPtr<D3D12BufferUav>(m_pContext, buffer, format, first_elem, num_elems);
}

RHIRenderTargetViewPtr D3D12Context::Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return MakeSharedPtr<D3D12Texture2DCubeRtv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
}
RHIRenderTargetViewPtr D3D12Context::Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    return MakeSharedPtr<D3D12TextureCubeFaceRtv>(m_pContext, tex_cube, array_index, face, mip_level);
}
RHIRenderTargetViewPtr D3D12Context::Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    return MakeSharedPtr<D3D12Texture3DRtv>(m_pContext, tex_3d, array_index, first_slice, num_slices, mip_level);
}
RHIDepthStencilViewPtr D3D12Context::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return MakeSharedPtr<D3D12Texture2DDsv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
}
RHIDepthStencilViewPtr D3D12Context::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    return MakeSharedPtr<D3D12TextureCubeFaceDsv>(m_pContext, tex_2d, array_index, face, mip_level);
}
RHIFrameBufferPtr D3D12Context::CreateRHIFrameBuffer()
{
    return nullptr;
}
RHIProgramPtr D3D12Context::CreateRHIProgram()
{
    return nullptr; 
}
RHITimeQueryPtr D3D12Context::CreateRHITimeQuery()
{
    return nullptr; 
}
RHIFencePtr D3D12Context::CreateFence()
{
    return MakeSharedPtr<D3D12Fence>(m_pContext);
}

SEEK_NAMESPACE_END