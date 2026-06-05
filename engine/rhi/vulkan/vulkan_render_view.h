#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkRenderTargetView — Vulkan RTV
// ============================================================================
class VkTexture2DCubeRtv : public RHIRenderTargetView
{
public:
    VkTexture2DCubeRtv(Context* context, RHITexturePtr const& tex, uint32_t first_array, uint32_t array_size, uint32_t mip);
    VkImageView GetVkImageView() const { return m_vkImageView; }

private:
    VkImageView m_vkImageView = VK_NULL_HANDLE;
};

class VkTextureCubeFaceRtv : public RHIRenderTargetView
{
public:
    VkTextureCubeFaceRtv(Context* context, RHITexturePtr const& tex, uint32_t array_index, CubeFaceType face, uint32_t mip);
    VkImageView GetVkImageView() const { return m_vkImageView; }

private:
    VkImageView m_vkImageView = VK_NULL_HANDLE;
};

class VkTexture3DRtv : public RHIRenderTargetView
{
public:
    VkTexture3DRtv(Context* context, RHITexturePtr const& tex, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip);
    VkImageView GetVkImageView() const { return m_vkImageView; }

private:
    VkImageView m_vkImageView = VK_NULL_HANDLE;
};

// ============================================================================
// VkDepthStencilView — Vulkan DSV
// ============================================================================
class VkTexture2DDsv : public RHIDepthStencilView
{
public:
    VkTexture2DDsv(Context* context, RHITexturePtr const& tex, uint32_t first_array, uint32_t array_size, uint32_t mip);
    VkImageView GetVkImageView() const { return m_vkImageView; }

private:
    VkImageView m_vkImageView = VK_NULL_HANDLE;
};

class VkTextureCubeFaceDsv : public RHIDepthStencilView
{
public:
    VkTextureCubeFaceDsv(Context* context, RHITexturePtr const& tex, uint32_t array_index, CubeFaceType face, uint32_t mip);
    VkImageView GetVkImageView() const { return m_vkImageView; }

private:
    VkImageView m_vkImageView = VK_NULL_HANDLE;
};

// ============================================================================
// VkShaderResourceView — Vulkan SRV
// ============================================================================
class VkBufferShaderResourceView : public RHIShaderResourceView
{
public:
    VkBufferShaderResourceView(Context* context, RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems);
    VkBufferView GetVkBufferView() const { return m_vkBufferView; }

private:
    VkBufferView m_vkBufferView = VK_NULL_HANDLE;
};

// ============================================================================
// VkUnorderedAccessView — Vulkan UAV
// ============================================================================
class VkBufferUnorderedAccessView : public RHIUnorderedAccessView
{
public:
    VkBufferUnorderedAccessView(Context* context, RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems);
    VkBufferView GetVkBufferView() const { return m_vkBufferView; }

private:
    VkBufferView m_vkBufferView = VK_NULL_HANDLE;
};

SEEK_NAMESPACE_END


