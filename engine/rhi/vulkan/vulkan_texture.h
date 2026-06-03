#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

SEEK_NAMESPACE_BEGIN

class VkContext;

class VkTexture2D : public RHITexture
{
public:
    VkTexture2D(Context* context, const Desc& desc);
    VkTexture2D(Context* context, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
    ~VkTexture2D() override;

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;

    SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;
    SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr) override;
    SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;

    VkImage         GetVkImage()       const { return m_vkImage; }
    VkImageView     GetVkImageView()   const { return m_vkImageView; }
    VkImageView     GetDefaultVkImageView() const { return m_vkDefaultImageView; }
    VkFormat        GetVkFormat()      const { return m_vkFormat; }
    VmaAllocation   GetVmaAllocation() const { return m_vmaAllocation; }

    void TransitionLayout(VkCommandBuffer cmdBuf, VkImageLayout oldLayout, VkImageLayout newLayout,
        uint32_t baseMip = 0, uint32_t levelCount = 1, uint32_t baseLayer = 0, uint32_t layerCount = 1);

protected:
    VkImage       m_vkImage = VK_NULL_HANDLE;
    VkImageView   m_vkImageView = VK_NULL_HANDLE;
    VkImageView   m_vkDefaultImageView = VK_NULL_HANDLE;
    VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
    VkFormat      m_vkFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_vkCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkContext*     GetVkContext() const;
    VkDevice       GetVkDevice() const;
    VmaAllocator   GetVmaAllocator() const;

private:
    SResult CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arraySize,
        VkSampleCountFlagBits samples, VkImageCreateFlags createFlags, VkImageUsageFlags usage);
    void     DestroyImage();
};

using VkTexture2DPtr = std::shared_ptr<VkTexture2D>;

class VkTexture3D : public RHITexture
{
public:
    VkTexture3D(Context* context, const Desc& desc);
    ~VkTexture3D() override;

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;

    SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;
    SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr) override;
    SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;

    VkImage         GetVkImage()       const { return m_vkImage; }
    VkImageView     GetVkImageView()   const { return m_vkImageView; }
    VkFormat        GetVkFormat()      const { return m_vkFormat; }
    VmaAllocation   GetVmaAllocation() const { return m_vmaAllocation; }

protected:
    VkImage       m_vkImage = VK_NULL_HANDLE;
    VkImageView   m_vkImageView = VK_NULL_HANDLE;
    VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
    VkFormat      m_vkFormat = VK_FORMAT_UNDEFINED;
};

using VkTexture3DPtr = std::shared_ptr<VkTexture3D>;

class VkTextureCube : public RHITexture
{
public:
    VkTextureCube(Context* context, const Desc& desc);
    ~VkTextureCube() override;

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;

    SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;
    SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr) override;
    SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;

    VkImage         GetVkImage()       const { return m_vkImage; }
    VkImageView     GetVkImageView()   const { return m_vkImageView; }
    VkFormat        GetVkFormat()      const { return m_vkFormat; }
    VmaAllocation   GetVmaAllocation() const { return m_vmaAllocation; }

protected:
    VkImage       m_vkImage = VK_NULL_HANDLE;
    VkImageView   m_vkImageView = VK_NULL_HANDLE;
    VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
    VkFormat      m_vkFormat = VK_FORMAT_UNDEFINED;
};

using VkTextureCubePtr = std::shared_ptr<VkTextureCube>;

SEEK_NAMESPACE_END

