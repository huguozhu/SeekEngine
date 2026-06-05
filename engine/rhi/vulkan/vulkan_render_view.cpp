#include "kernel/context.h"
#include "rhi/vulkan/vulkan_render_view.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkTexture2DCubeRtv
// ============================================================================
VkTexture2DCubeRtv::VkTexture2DCubeRtv(Context* context, RHITexturePtr const& tex,
    uint32_t first_array, uint32_t array_size, uint32_t mip)
    : RHIRenderTargetView(context)
{
    m_Param.texture = tex;
    m_Param.first_array_index = first_array;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = mip;
    m_iWidth = tex->Width();
    m_iHeight = tex->Height();
    m_Param.pixel_format = tex->Descriptor().format;
    m_iNumSamples = tex->Descriptor().num_samples;
    VkTexture2D* vkTex = static_cast<VkTexture2D*>(tex.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());

    if (vkTex && vkTex->GetVkImage() != VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkTex->GetVkImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkTex->GetVkFormat();
        viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(viewInfo.format);
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = first_array;
        viewInfo.subresourceRange.layerCount = array_size;

        vkCreateImageView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkImageView);
    }
}

// ============================================================================
// VkTextureCubeFaceRtv
// ============================================================================
VkTextureCubeFaceRtv::VkTextureCubeFaceRtv(Context* context, RHITexturePtr const& tex,
    uint32_t array_index, CubeFaceType face, uint32_t mip)
    : RHIRenderTargetView(context)
{
    m_Param.texture = tex;
    m_Param.first_array_index = array_index;
    m_Param.first_face = face;
    m_Param.num_faces = 1;
    m_Param.mip_level = mip;
    m_iWidth = tex->Width(mip);
    m_iHeight = tex->Height(mip);
    m_Param.pixel_format = tex->Descriptor().format;
    m_iNumSamples = tex->Descriptor().num_samples;

    VkTextureCube* vkTex = static_cast<VkTextureCube*>(tex.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());

    if (vkTex && vkTex->GetVkImage() != VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkTex->GetVkImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkTex->GetVkFormat();
        viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(viewInfo.format);
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = array_index * 6 + static_cast<uint32_t>(face);
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkImageView);
    }
}

// ============================================================================
// VkTexture3DRtv
// ============================================================================
VkTexture3DRtv::VkTexture3DRtv(Context* context, RHITexturePtr const& tex,
    uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip)
    : RHIRenderTargetView(context)
{
    m_Param.texture = tex;
    m_Param.first_array_index = array_index;
    m_Param.first_slice = first_slice;
    m_Param.num_slices = num_slices;
    m_Param.mip_level = mip;
    m_iWidth = tex->Width(mip);
    m_iHeight = tex->Height(mip);
    m_Param.pixel_format = tex->Descriptor().format;
    m_iNumSamples = tex->Descriptor().num_samples;

    VkTexture3D* vkTex = static_cast<VkTexture3D*>(tex.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());

    if (vkTex && vkTex->GetVkImage() != VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkTex->GetVkImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.format = vkTex->GetVkFormat();
        viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(viewInfo.format);
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkImageView);
    }
}

// ============================================================================
// VkTexture2DDsv
// ============================================================================
VkTexture2DDsv::VkTexture2DDsv(Context* context, RHITexturePtr const& tex,
    uint32_t first_array, uint32_t array_size, uint32_t mip)
    : RHIDepthStencilView(context)
{
    m_Param.texture = tex;
    m_Param.first_array_index = first_array;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = mip;
    m_iWidth = tex->Width();
    m_iHeight = tex->Height();
    m_Param.pixel_format = tex->Descriptor().format;
    m_iNumSamples = tex->Descriptor().num_samples;

    VkTexture2D* vkTex = static_cast<VkTexture2D*>(tex.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());

    if (vkTex && vkTex->GetVkImage() != VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkTex->GetVkImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkTex->GetVkFormat();
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = first_array;
        viewInfo.subresourceRange.layerCount = array_size;

        vkCreateImageView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkImageView);
    }
}

// ============================================================================
// VkTextureCubeFaceDsv
// ============================================================================
VkTextureCubeFaceDsv::VkTextureCubeFaceDsv(Context* context, RHITexturePtr const& tex,
    uint32_t array_index, CubeFaceType face, uint32_t mip)
    : RHIDepthStencilView(context)
{
    m_Param.texture = tex;
    m_Param.first_array_index = array_index;
    m_Param.first_face = face;
    m_Param.num_faces = 1;
    m_Param.mip_level = mip;
    m_iWidth = tex->Width(mip);
    m_iHeight = tex->Height(mip);
    m_Param.pixel_format = tex->Descriptor().format;
    m_iNumSamples = tex->Descriptor().num_samples;

    VkTextureCube* vkTex = static_cast<VkTextureCube*>(tex.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());

    if (vkTex && vkTex->GetVkImage() != VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkTex->GetVkImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkTex->GetVkFormat();
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = array_index * 6 + static_cast<uint32_t>(face);
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkImageView);
    }
}

// ============================================================================
// VkBufferShaderResourceView
// ============================================================================
VkBufferShaderResourceView::VkBufferShaderResourceView(Context* context,
    RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
    : RHIShaderResourceView(context)
{
    m_Param.buffer = buffer;
    m_Param.pixel_format = format;
    m_Param.first_elem = first_elem;
    m_Param.num_elem = num_elems;

    VkGpuBuffer* vkBuf = static_cast<VkGpuBuffer*>(buffer.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());
    VkFormat vkFormat = VkTranslate::PixelFormatToVkFormat(format);

    if (vkBuf && vkBuf->GetVkBuffer() != VK_NULL_HANDLE && vkFormat != VK_FORMAT_UNDEFINED)
    {
        uint32_t elemSize = Formatutil::NumComponentBytes(format);
        VkBufferViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        viewInfo.buffer = vkBuf->GetVkBuffer();
        viewInfo.format = vkFormat;
        viewInfo.offset = first_elem * elemSize;
        viewInfo.range = num_elems * elemSize;

        vkCreateBufferView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkBufferView);
    }
}

// ============================================================================
// VkBufferUnorderedAccessView
// ============================================================================
VkBufferUnorderedAccessView::VkBufferUnorderedAccessView(Context* context,
    RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems)
    : RHIUnorderedAccessView(context)
{
    m_Param.buffer = buffer;
    m_Param.pixel_format = format;
    m_Param.first_elem = first_elem;
    m_Param.num_elem = num_elems;

    VkGpuBuffer* vkBuf = static_cast<VkGpuBuffer*>(buffer.get());
    VkContext* vkCtx = static_cast<VkContext*>(&context->RHIContextInstance());
    VkFormat vkFormat = VkTranslate::PixelFormatToVkFormat(format);

    if (vkBuf && vkBuf->GetVkBuffer() != VK_NULL_HANDLE && vkFormat != VK_FORMAT_UNDEFINED)
    {
        uint32_t elemSize = Formatutil::NumComponentBytes(format);
        VkBufferViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        viewInfo.buffer = vkBuf->GetVkBuffer();
        viewInfo.format = vkFormat;
        viewInfo.offset = first_elem * elemSize;
        viewInfo.range = num_elems * elemSize;

        vkCreateBufferView(vkCtx->GetVkDevice(), &viewInfo, nullptr, &m_vkBufferView);
    }
}

SEEK_NAMESPACE_END

