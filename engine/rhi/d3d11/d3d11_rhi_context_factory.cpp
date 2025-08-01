#include "rhi/d3d11/d3d11_predeclare.h"
#include "rhi/d3d11/d3d11_rhi_context.h"
#include "rhi/d3d11/d3d11_window.h"
#include "rhi/d3d11/d3d11_texture.h"
#include "rhi/d3d11/d3d11_gpu_buffer.h"
#include "rhi/d3d11/d3d11_render_state.h"
#include "rhi/d3d11/d3d11_program.h"
#include "rhi/d3d11/d3d11_shader.h"
#include "rhi/d3d11/d3d11_mesh.h"
#include "rhi/d3d11/d3d11_render_view.h"
#include "rhi/d3d11/d3d11_fence.h"

#define SEEK_MACRO_FILE_UID 8     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

RHIMeshPtr D3D11Context::CreateMesh()
{
    RHIMeshPtr res = MakeSharedPtr<D3D11Mesh>(m_pContext);
    return res;
}

RHIShaderPtr D3D11Context::CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
{
    return MakeSharedPtr<D3D11Shader>(m_pContext, type, name, entry_func_name, code);
}

RHITexturePtr D3D11Context::CreateTexture2D(ID3D11Texture2DPtr const& tex)
{
    return MakeSharedPtr<D3D11Texture2D>(m_pContext, tex);
}
RHITexturePtr D3D11Context::CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture2DPtr tex = MakeSharedPtr<D3D11Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11Context::CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture3DPtr tex = MakeSharedPtr<D3D11Texture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11Context::CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11TextureCubePtr tex = MakeSharedPtr<D3D11TextureCube>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHIShaderResourceViewPtr D3D11Context::CreateBufferSrv(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
    return MakeSharedPtr<D3D11BufferShaderResourceView>(m_pContext, buffer, format, first_elem, num_elems);
}
RHIUnorderedAccessViewPtr D3D11Context::CreateBufferUav(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
    return MakeSharedPtr<D3D11BufferUnorderedAccessView>(m_pContext, buffer, format, first_elem, num_elems);
}
RHIGpuBufferPtr D3D11Context::CreateGpuBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_stride, RHIGpuBufferData* pData)
{
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D11GpuBuffer>(m_pContext, size, flags, GpuBufferType::COMMON_BUFFER, structure_stride);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D11Context::CreateConstantBuffer(uint32_t size, ResourceFlags flags, RHIGpuBufferData* pData)
{
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D11ConstantBuffer>(m_pContext, seek_alignup(size, 16), flags);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D11Context::CreateVertexBuffer(uint32_t size, RHIGpuBufferData* pData)
{
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D11VertexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}
RHIGpuBufferPtr D3D11Context::CreateIndexBuffer(uint32_t size, RHIGpuBufferData* pData)
{
    RHIGpuBufferPtr buf = MakeSharedPtr<D3D11IndexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}

RHIRenderStatePtr D3D11Context::CreateRenderState(RenderStateDesc const& desc)
{
    RHIRenderStatePtr state = MakeSharedPtr<D3D11RenderState>(m_pContext, desc);
    return state;
}
RHISamplerPtr D3D11Context::CreateSampler(SamplerDesc const& desc)
{
    RHISamplerPtr sampler = MakeSharedPtr<D3D11Sampler>(m_pContext, desc);
    return sampler;
}
RHIRenderTargetViewPtr D3D11Context::Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11Texture2DCubeRtv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
    return rtv;
}
RHIRenderTargetViewPtr D3D11Context::Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11TextureCubeFaceRtv>(m_pContext, tex_cube, array_index, face, mip_level);
    return rtv;
}
RHIRenderTargetViewPtr D3D11Context::Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    RHIRenderTargetViewPtr rtv = MakeSharedPtr<D3D11Texture3DRtv>(m_pContext, tex_3d, array_index, first_slice, num_slices, mip_level);
    return rtv;
}
RHIDepthStencilViewPtr D3D11Context::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    RHIDepthStencilViewPtr dsv = MakeSharedPtr<D3D11Texture2DDsv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
    return dsv;
}
RHIDepthStencilViewPtr D3D11Context::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    RHIDepthStencilViewPtr dsv = MakeSharedPtr<D3D11TextureCubeFaceDsv>(m_pContext, tex_2d, array_index, face, mip_level);
    return dsv;
}
RHIFrameBufferPtr D3D11Context::CreateRHIFrameBuffer()
{
    RHIFrameBufferPtr fb = MakeSharedPtr<D3D11FrameBuffer>(m_pContext);
    return fb;
}
RHIProgramPtr D3D11Context::CreateRHIProgram()
{ 
    return MakeSharedPtr<D3D11Program>(m_pContext);
}
RHITimeQueryPtr D3D11Context::CreateRHITimeQuery()
{
    return MakeSharedPtr<D3D11TimeQuery>(m_pContext);
}
RHIFencePtr D3D11Context::CreateFence()
{
    return MakeSharedPtr<D3D11Fence>(m_pContext);
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
