#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_render_state.h"
#include "rhi/vulkan/vulkan_program.h"
#include "rhi/vulkan/vulkan_shader.h"
#include "rhi/vulkan/vulkan_mesh.h"
#include "rhi/vulkan/vulkan_render_view.h"
#include "rhi/vulkan/vulkan_fence.h"
#include "rhi/vulkan/vulkan_query.h"

#define SEEK_MACRO_FILE_UID 71     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

// ============================================================================
// Vulkan RHI 工厂方法 — 创建各种 Vulkan RHI 对象
// ============================================================================

RHIMeshPtr VkContext::CreateMesh()
{
    return MakeSharedPtr<VkMesh>(m_pContext);
}

RHIShaderPtr VkContext::CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
{
    return MakeSharedPtr<VkShader>(m_pContext, type, name, entry_func_name, code);
}

RHITexturePtr VkContext::CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex2D || tex_desc.width <= 0 || tex_desc.height <= 0 || tex_desc.num_mips <= 0)
        return nullptr;

    auto tex = MakeSharedPtr<VkTexture2D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}

RHITexturePtr VkContext::CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas)
{
    if (tex_desc.type != TextureType::Tex3D || tex_desc.width <= 0 || tex_desc.height <= 0 || tex_desc.num_mips <= 0)
        return nullptr;

    auto tex = MakeSharedPtr<VkTexture3D>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_datas);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}

RHITexturePtr VkContext::CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data)
{
    if (tex_desc.type != TextureType::Cube || tex_desc.width <= 0 || tex_desc.height <= 0 || tex_desc.num_mips <= 0)
        return nullptr;

    auto tex = MakeSharedPtr<VkTextureCube>(m_pContext, tex_desc);
    SResult ret = tex->Create(init_data);
    return SEEK_CHECKFAILED(ret) ? nullptr : tex;
}

RHIGpuBufferPtr VkContext::CreateGpuBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_stride, RHIGpuBufferData* pData)
{
    auto buf = MakeSharedPtr<VkGpuBuffer>(m_pContext, size, flags, GpuBufferType::COMMON_BUFFER, structure_stride);
    buf->Create(pData);
    return buf;
}

RHIGpuBufferPtr VkContext::CreateConstantBuffer(uint32_t size, ResourceFlags flags, RHIGpuBufferData* pData)
{
    auto buf = MakeSharedPtr<VkConstantBuffer>(m_pContext, size, flags);
    buf->Create(pData);
    return buf;
}

RHIGpuBufferPtr VkContext::CreateVertexBuffer(uint32_t size, RHIGpuBufferData* pData)
{
    auto buf = MakeSharedPtr<VkVertexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}

RHIGpuBufferPtr VkContext::CreateIndexBuffer(uint32_t size, RHIGpuBufferData* pData)
{
    auto buf = MakeSharedPtr<VkIndexBuffer>(m_pContext, size);
    buf->Create(pData);
    return buf;
}

RHIShaderResourceViewPtr VkContext::CreateBufferSrv(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
    return MakeSharedPtr<VkBufferShaderResourceView>(m_pContext, buffer, format, first_elem, num_elems);
}

RHIUnorderedAccessViewPtr VkContext::CreateBufferUav(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
    return MakeSharedPtr<VkBufferUnorderedAccessView>(m_pContext, buffer, format, first_elem, num_elems);
}

RHIRenderTargetViewPtr VkContext::Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return MakeSharedPtr<VkTexture2DCubeRtv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
}

RHIRenderTargetViewPtr VkContext::Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    return MakeSharedPtr<VkTextureCubeFaceRtv>(m_pContext, tex_cube, array_index, face, mip_level);
}

RHIRenderTargetViewPtr VkContext::Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    return MakeSharedPtr<VkTexture3DRtv>(m_pContext, tex_3d, array_index, first_slice, num_slices, mip_level);
}

RHIDepthStencilViewPtr VkContext::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return MakeSharedPtr<VkTexture2DDsv>(m_pContext, tex_2d, first_array_index, array_size, mip_level);
}

RHIDepthStencilViewPtr VkContext::Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    return MakeSharedPtr<VkTextureCubeFaceDsv>(m_pContext, tex_2d, array_index, face, mip_level);
}

RHIFrameBufferPtr VkContext::CreateRHIFrameBuffer()
{
    return MakeSharedPtr<VkFrameBuffer>(m_pContext);
}

RHIProgramPtr VkContext::CreateRHIProgram()
{
    return MakeSharedPtr<VkProgram>(m_pContext);
}

RHITimeQueryPtr VkContext::CreateRHITimeQuery()
{
    return MakeSharedPtr<VkTimeQuery>(m_pContext);
}

RHIFencePtr VkContext::CreateFence()
{
    return MakeSharedPtr<VkFence>(m_pContext);
}

RHIRenderStatePtr VkContext::CreateRenderState(RenderStateDesc const& desc)
{
    return MakeSharedPtr<VkRenderState>(m_pContext, desc);
}

RHISamplerPtr VkContext::CreateSampler(SamplerDesc const& desc)
{
    return MakeSharedPtr<VkSampler>(m_pContext, desc);
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
