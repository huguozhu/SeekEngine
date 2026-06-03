#include "kernel/context.h"
#include "rhi/vulkan/vulkan_framebuffer.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_render_view.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 77     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

VkFrameBuffer::VkFrameBuffer(Context* context)
    : RHIFrameBuffer(context)
{
}

VkFrameBuffer::~VkFrameBuffer()
{
    Destroy();
}

void VkFrameBuffer::Destroy()
{
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    if (m_vkFramebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, m_vkFramebuffer, nullptr);
        m_vkFramebuffer = VK_NULL_HANDLE;
    }
    if (m_vkRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, m_vkRenderPass, nullptr);
        m_vkRenderPass = VK_NULL_HANDLE;
    }
}

SResult VkFrameBuffer::OnBind()
{
    // 缁戝畾 framebuffer 鏃剁殑鎿嶄綔
    return S_Success;
}

SResult VkFrameBuffer::OnUnbind()
{
    // 瑙ｇ粦 framebuffer 鏃剁殑鎿嶄綔
    return S_Success;
}

SResult VkFrameBuffer::Build()
{
    // Simplified: off-screen framebuffer build
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    std::vector<VkImageView> attachmentViews;

    for (int i = 0; i < 8; i++)
    {
        RHIFrameBuffer::Attachment att = static_cast<RHIFrameBuffer::Attachment>(i);
        RHIRenderTargetViewPtr rtv = GetRenderTarget(att);
        if (!rtv) continue;
        if (auto* vkRtv = static_cast<VkTexture2DCubeRtv*>(rtv.get()))
            attachmentViews.push_back(vkRtv->GetVkImageView());
    }

    if (auto dsv = GetDepthStencilView())
    {
        if (auto* vkDsv = static_cast<VkTexture2DDsv*>(dsv.get()))
            attachmentViews.push_back(vkDsv->GetVkImageView());
    }

    if (attachmentViews.empty()) { m_uWidth = 1; m_uHeight = 1; return S_Success; }
    m_uWidth = 256; m_uHeight = 256;

    VkAttachmentDescription colorAtt = {};
    colorAtt.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkRenderPassCreateInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &colorAtt;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &rpInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS)
        return ERR_SYSTEM_ERROR;

    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = m_vkRenderPass;
    fbInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    fbInfo.pAttachments = attachmentViews.data();
    fbInfo.width = m_uWidth;
    fbInfo.height = m_uHeight;
    fbInfo.layers = 1;

    if (vkCreateFramebuffer(device, &fbInfo, nullptr, &m_vkFramebuffer) != VK_SUCCESS)
        return ERR_SYSTEM_ERROR;

    return S_Success;
}

SEEK_NAMESPACE_END




