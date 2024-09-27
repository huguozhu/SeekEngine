#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_window.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_render_buffer.h"
#include "rhi/d3d11_rhi/d3d11_render_state.h"
#include "rhi/d3d11_rhi/d3d11_program.h"
#include "rhi/d3d11_rhi/d3d11_shader.h"
#include "rhi/d3d11_rhi/d3d11_mesh.h"
#include "rhi/d3d11_rhi/d3d11_render_view.h"

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

RHITexturePtr D3D11RHIContext::CreateTexture2D(const RHITexture::Desc& tex_desc, const BitmapBufferPtr init_data)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;

    RHITexturePtr tex = MakeSharedPtr<D3D11Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(std::vector<BitmapBufferPtr>{init_data});
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11RHIContext::CreateTexture2D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture2DPtr tex = MakeSharedPtr<D3D11Texture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11RHIContext::CreateTexture3D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11Texture3DPtr tex = MakeSharedPtr<D3D11Texture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHITexturePtr D3D11RHIContext::CreateTextureCube(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr>* init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width < 0 || tex_desc.height < 0 || tex_desc.num_mips <= 0)
        return nullptr;
    D3D11TextureCubePtr tex = MakeSharedPtr<D3D11TextureCube>(m_pContext, tex_desc);
    SResult ret = tex->CreateCube(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}
RHIRenderBufferPtr D3D11RHIContext::CreateVertexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11VertexBuffer>(m_pContext, size, flags);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateIndexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11IndexBuffer>(m_pContext, size, flags);
    buf->Create(pData);
    return buf;
}
RHIRenderBufferPtr D3D11RHIContext::CreateRHIRenderBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData)
{
    RHIRenderBufferPtr buf = MakeSharedPtr<D3D11RHIRenderBuffer>(m_pContext, size, flags);
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

RenderViewPtr D3D11RHIContext::CreateRenderTargetView(RHITexturePtr const& tex, uint32_t lod)
{
    RenderViewPtr rtv = MakeSharedPtr<D3D11RenderTargetView>(m_pContext, tex, lod);
    return rtv;
}
RenderViewPtr D3D11RHIContext::CreateRenderTargetView(RHITexturePtr const& tex, CubeFaceType face, uint32_t lod)
{
    RenderViewPtr rtv = MakeSharedPtr<D3D11CubeFaceRenderTargetView>(m_pContext, tex, face, lod);
    return rtv;
}
RenderViewPtr D3D11RHIContext::CreateDepthStencilView(RHITexturePtr const& tex)
{
    RenderViewPtr dsv = MakeSharedPtr<D3D11DepthStencilView>(m_pContext, tex);
    return dsv;
}
RenderViewPtr D3D11RHIContext::CreateDepthStencilView(RHITexturePtr const& tex, CubeFaceType face)
{
    RenderViewPtr rtv = MakeSharedPtr<D3D11CubeDepthStencilView>(m_pContext, tex, face);
    return rtv;
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
RHITimerQueryPtr D3D11RHIContext::CreateRHITimerQuery()
{
    return MakeSharedPtr<D3D11RHITimerQuery>(m_pContext);
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
