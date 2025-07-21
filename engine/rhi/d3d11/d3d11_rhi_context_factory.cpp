#include "rhi/d3d11/d3d11_predeclare.h"
#include "rhi/d3d11/d3d11_rhi_context.h"
#include "rhi/d3d11/d3d11_window.h"
#include "rhi/d3d11/d3d11_texture.h"
#include "rhi/d3d11/d3d11_render_buffer.h"
#include "rhi/d3d11/d3d11_render_state.h"
#include "rhi/d3d11/d3d11_program.h"
#include "rhi/d3d11/d3d11_shader.h"
#include "rhi/d3d11/d3d11_mesh.h"
#include "rhi/d3d11/d3d11_render_view.h"
#include "rhi/d3d11/d3d11_fence.h"

#define SEEK_MACRO_FILE_UID 8     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

RHIMeshPtr D3D11RHIContext::CreateMesh()
{
    RHIMeshPtr res = MakeSharedPtr<D3D11Mesh>(m_pContext);
    return res;
}

RHIShaderPtr D3D11RHIContext::CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
{
    return MakeSharedPtr<D3D11Shader>(m_pContext, type, name, entry_func_name, code);
}

RHITexturePtr D3D11RHIContext::CreateTexture2D(ID3D11Texture2DPtr const& tex)
{
    return MakeSharedPtr<D3D11Texture2D>(m_pContext, tex);
}
RHITexturePtr D3D11RHIContext::CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture2DPtr tex = MakeSharedPtr<D3D11Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11RHIContext::CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture3DPtr tex = MakeSharedPtr<D3D11Texture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11RHIContext::CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11TextureCubePtr tex = MakeSharedPtr<D3D11TextureCube>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHIRenderBufferPtr D3D11RHIContext::CreateVertexBuffer(uint32_t size, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11VertexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateIndexBuffer(uint32_t size, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11IndexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateConstantBuffer(uint32_t size, ResourceFlags flags)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11ConstantBuffer>(m_pContext, size, flags);
    buf->Create(nullptr);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11StructuredBuffer>(m_pContext, size, flags, structure_byte_stride);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateRWStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData)
{
    flags |= RESOURCE_FLAG_GPU_WRITE;
    return this->CreateStructuredBuffer(size, flags, structure_byte_stride, pData);
}
RHIRenderBufferPtr D3D11RHIContext::CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11ByteAddressBuffer>(m_pContext, size, flags);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    flags |= RESOURCE_FLAG_GPU_WRITE;
    return CreateByteAddressBuffer(size, flags, pData);
}
RHIRenderStatePtr D3D11RHIContext::CreateRenderState(RenderStateDesc const& desc)
{
    RHIRenderStatePtr state = MakeSharedPtr<D3D11RenderState>(m_pContext, desc);
    return state;
}
RHISamplerPtr D3D11RHIContext::CreateSampler(SamplerDesc const& desc)
{
    RHISamplerPtr sampler = MakeSharedPtr<D3D11Sampler>(m_pContext, desc);
    return sampler;
}
RHIRenderTargetViewPtr D3D11RHIContext::Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11Texture2DCubeRtv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
    return rtv;
}
RHIRenderTargetViewPtr D3D11RHIContext::Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11TextureCubeFaceRtv>(m_pContext, tex_cube, array_index, face, mip_level);
    return rtv;
}
RHIRenderTargetViewPtr D3D11RHIContext::Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11Texture3DRtv>(m_pContext, tex_3d, array_index, first_slice, num_slices, mip_level);
    return rtv;
}
RHIDepthStencilViewPtr D3D11RHIContext::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    RHIDepthStencilViewPtr dsv = MakeSharedPtr<D3D11Texture2DDsv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
    return dsv;
}
RHIDepthStencilViewPtr D3D11RHIContext::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    RHIDepthStencilViewPtr dsv = MakeSharedPtr<D3D11TextureCubeFaceDsv>(m_pContext, tex_2d, array_index, face, mip_level);
    return dsv;
}
RHIFrameBufferPtr D3D11RHIContext::CreateRHIFrameBuffer()
{
    RHIFrameBufferPtr fb = MakeSharedPtr<D3D11RHIFrameBuffer>(m_pContext);
    return fb;
}
RHIProgramPtr D3D11RHIContext::CreateRHIProgram()
{ 
    return MakeSharedPtr<D3D11RHIProgram>(m_pContext);
}
RHITimeQueryPtr D3D11RHIContext::CreateRHITimeQuery()
{
    return MakeSharedPtr<D3D11RHITimeQuery>(m_pContext);
}
RHIFencePtr D3D11RHIContext::CreateFence()
{
    return MakeSharedPtr<D3D11RHIFence>(m_pContext);
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
