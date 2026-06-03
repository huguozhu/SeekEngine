#include "kernel/context.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "utils/log.h"
#include "utils/buffer.h"

#define SEEK_MACRO_FILE_UID 74     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkTexture2D
// ============================================================================
VkContext* VkTexture2D::GetVkContext() const { return static_cast<VkContext*>(&m_pContext->RHIContextInstance()); }
VkDevice VkTexture2D::GetVkDevice() const { return GetVkContext()->GetVkDevice(); }
VmaAllocator VkTexture2D::GetVmaAllocator() const { return GetVkContext()->GetVmaAllocator(); }

VkTexture2D::VkTexture2D(Context* context, const Desc& desc)
    : RHITexture(context, desc)
{
}

VkTexture2D::VkTexture2D(Context* context, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels)
    : RHITexture(context, Desc{})
{
    m_vkImage = image;
    m_vkFormat = format;
    m_desc.type = TextureType::Tex2D;
    m_desc.width = width;
    m_desc.height = height;
    m_desc.num_mips = mipLevels;
    m_vkCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

VkTexture2D::~VkTexture2D()
{
    DestroyImage();
}

SResult VkTexture2D::Create(std::span<BitmapBufferPtr> const& init_datas)
{
    if (m_desc.width <= 0 || m_desc.height <= 0)
        return ERR_INVALID_ARG;

    m_vkFormat = VkTranslate::PixelFormatToVkFormat(m_desc.format);
    if (m_vkFormat == VK_FORMAT_UNDEFINED)
    {
        LOG_ERROR("Unsupported pixel format for Vulkan texture");
        return ERR_NOT_IMPLEMENTED;
    }

    uint32_t mipLevels = m_desc.num_mips;
    if (mipLevels <= 0) mipLevels = 1;

    VkImageUsageFlags usage = VkTranslate::GetVkImageUsageFlags(m_desc.flags, m_desc.format);
    usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkSampleCountFlagBits samples = VkTranslate::SampleCountToVkFlags(m_desc.num_samples);

    SResult ret = CreateImage(m_desc.width, m_desc.height, mipLevels, m_desc.num_array,
        samples, 0, usage);
    if (SEEK_CHECKFAILED(ret))
        return ret;

    // 上传初始数据
    if (!init_datas.empty() && init_datas[0])
    {
        VkContext* vkCtx = GetVkContext();
        VkCommandBuffer cmdBuf = vkCtx->BeginSingleTimeCommands();

        TransitionLayout(cmdBuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        size_t dataSize = init_datas[0]->Size();
        // Staging buffer upload
        VkBuffer stagingBuffer;
        VmaAllocation stagingAlloc;
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = dataSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (vmaCreateBuffer(GetVmaAllocator(), &bufferInfo, &allocInfo,
            &stagingBuffer, &stagingAlloc, nullptr) == VK_SUCCESS)
        {
            void* mappedData;
            vmaMapMemory(GetVmaAllocator(), stagingAlloc, &mappedData);
            memcpy(mappedData, init_datas[0]->Data(), dataSize);
            vmaUnmapMemory(GetVmaAllocator(), stagingAlloc);

            VkBufferImageCopy region = {};
            region.bufferOffset = 0;
            region.imageSubresource.aspectMask = VkTranslate::GetImageAspectFromVkFormat(m_vkFormat);
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = { (uint32_t)m_desc.width, (uint32_t)m_desc.height, 1 };

            vkCmdCopyBufferToImage(cmdBuf, stagingBuffer, m_vkImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            vkCtx->EndSingleTimeCommands(cmdBuf);

            vmaDestroyBuffer(GetVmaAllocator(), stagingBuffer, stagingAlloc);
        }
        else
        {
            vkCtx->EndSingleTimeCommands(cmdBuf);
        }
    }

    // 创建默认 image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_vkFormat;
    viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(m_vkFormat);
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(GetVkDevice(), &viewInfo, nullptr, &m_vkDefaultImageView);

    return S_Success;
}

SResult VkTexture2D::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arraySize,
    VkSampleCountFlagBits samples, VkImageCreateFlags createFlags, VkImageUsageFlags usage)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = m_vkFormat;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arraySize;
    imageInfo.samples = samples;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = createFlags;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult result = vmaCreateImage(GetVmaAllocator(), &imageInfo, &allocInfo, &m_vkImage, &m_vmaAllocation, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vmaCreateImage failed: %d", result);
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

void VkTexture2D::DestroyImage()
{
    VkDevice device = GetVkDevice();
    if (m_vkDefaultImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, m_vkDefaultImageView, nullptr);
        m_vkDefaultImageView = VK_NULL_HANDLE;
    }
    if (m_vkImage != VK_NULL_HANDLE && m_vmaAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyImage(GetVmaAllocator(), m_vkImage, m_vmaAllocation);
        m_vkImage = VK_NULL_HANDLE;
        m_vmaAllocation = VK_NULL_HANDLE;
    }
}

void VkTexture2D::TransitionLayout(VkCommandBuffer cmdBuf, VkImageLayout oldLayout, VkImageLayout newLayout,
    uint32_t baseMip, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount)
{
    if (oldLayout == newLayout) return;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_vkImage;
    barrier.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(m_vkFormat);
    barrier.subresourceRange.baseMipLevel = baseMip;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.layerCount = layerCount;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(cmdBuf, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    m_vkCurrentLayout = newLayout;
}

SResult VkTexture2D::Update(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    // 简化实现：staging buffer 上传
    if (bitmap_datas.empty() || !bitmap_datas[0]) return S_Success;
    VkContext* vkCtx = GetVkContext();
    VkCommandBuffer cmdBuf = vkCtx->BeginSingleTimeCommands();
        // 上传数据...
    vkCtx->EndSingleTimeCommands(cmdBuf);
    return S_Success;
}

SResult VkTexture2D::DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{
    // TODO: 从 GPU 读回纹理数据
    return S_Success;
}

SResult VkTexture2D::DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Box<uint32_t>* box)
{
    return S_Success;
}

SResult VkTexture2D::DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{
    return S_Success;
}

// ============================================================================
// VkTexture3D
// ============================================================================
VkTexture3D::VkTexture3D(Context* context, const Desc& desc)
    : RHITexture(context, desc)
{
}

VkTexture3D::~VkTexture3D()
{
    if (m_vkImageView != VK_NULL_HANDLE) vkDestroyImageView(
        static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVkDevice(), m_vkImageView, nullptr);
    if (m_vkImage != VK_NULL_HANDLE && m_vmaAllocation != VK_NULL_HANDLE)
        vmaDestroyImage(static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVmaAllocator(),
            m_vkImage, m_vmaAllocation);
}

SResult VkTexture3D::Create(std::span<BitmapBufferPtr> const& init_datas)
{
    m_vkFormat = VkTranslate::PixelFormatToVkFormat(m_desc.format);
    if (m_vkFormat == VK_FORMAT_UNDEFINED) return ERR_NOT_IMPLEMENTED;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.format = m_vkFormat;
    imageInfo.extent = { (uint32_t)m_desc.width, (uint32_t)m_desc.height, (uint32_t)m_desc.depth };
    imageInfo.mipLevels = std::max(m_desc.num_mips, 1u);
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaAllocator vma = static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVmaAllocator();
    vmaCreateImage(vma, &imageInfo, &allocInfo, &m_vkImage, &m_vmaAllocation, nullptr);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    viewInfo.format = m_vkFormat;
    viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(m_vkFormat);
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVkDevice(),
        &viewInfo, nullptr, &m_vkImageView);

    return S_Success;
}

SResult VkTexture3D::Update(std::span<BitmapBufferPtr> const&) { return S_Success; }
SResult VkTexture3D::DumpSubResource2D(BitmapBufferPtr, uint32_t, uint32_t, Rect<uint32_t>*) { return S_Success; }
SResult VkTexture3D::DumpSubResource3D(BitmapBufferPtr, uint32_t, uint32_t, Box<uint32_t>*) { return S_Success; }
SResult VkTexture3D::DumpSubResourceCube(BitmapBufferPtr, CubeFaceType, uint32_t, uint32_t, Rect<uint32_t>*) { return S_Success; }

// ============================================================================
// VkTextureCube
// ============================================================================
VkTextureCube::VkTextureCube(Context* context, const Desc& desc)
    : RHITexture(context, desc)
{
}

VkTextureCube::~VkTextureCube()
{
    if (m_vkImageView != VK_NULL_HANDLE)
        vkDestroyImageView(static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVkDevice(), m_vkImageView, nullptr);
    if (m_vkImage != VK_NULL_HANDLE && m_vmaAllocation != VK_NULL_HANDLE)
        vmaDestroyImage(static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVmaAllocator(),
            m_vkImage, m_vmaAllocation);
}

SResult VkTextureCube::Create(std::span<BitmapBufferPtr> const& init_datas)
{
    m_vkFormat = VkTranslate::PixelFormatToVkFormat(m_desc.format);
    if (m_vkFormat == VK_FORMAT_UNDEFINED) return ERR_NOT_IMPLEMENTED;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = m_vkFormat;
    imageInfo.extent = { (uint32_t)m_desc.width, (uint32_t)m_desc.height, 1 };
    imageInfo.mipLevels = std::max(m_desc.num_mips, 1u);
    imageInfo.arrayLayers = 6;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaAllocator vma = static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVmaAllocator();
    vmaCreateImage(vma, &imageInfo, &allocInfo, &m_vkImage, &m_vmaAllocation, nullptr);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = m_vkFormat;
    viewInfo.subresourceRange.aspectMask = VkTranslate::GetImageAspectFromVkFormat(m_vkFormat);
    viewInfo.subresourceRange.levelCount = std::max(m_desc.num_mips, 1u);
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(static_cast<VkContext*>(&m_pContext->RHIContextInstance())->GetVkDevice(),
        &viewInfo, nullptr, &m_vkImageView);

    return S_Success;
}

SResult VkTextureCube::Update(std::span<BitmapBufferPtr> const&) { return S_Success; }
SResult VkTextureCube::DumpSubResource2D(BitmapBufferPtr, uint32_t, uint32_t, Rect<uint32_t>*) { return S_Success; }
SResult VkTextureCube::DumpSubResource3D(BitmapBufferPtr, uint32_t, uint32_t, Box<uint32_t>*) { return S_Success; }
SResult VkTextureCube::DumpSubResourceCube(BitmapBufferPtr, CubeFaceType, uint32_t, uint32_t, Rect<uint32_t>*) { return S_Success; }

SEEK_NAMESPACE_END



