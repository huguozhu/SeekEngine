#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#pragma once

#include "kernel/kernel.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

// Vulkan 设备相关对象智能指针（使用 shared_ptr + 自定义 deleter）
// VkInstance / VkDevice / VkQueue 等非 dispatchable 对象通过 Context 成员管理

// VkImageView / VkSampler 等 dispatchable 小对象用 shared_ptr 包装

struct VkImageDeleter { void operator()(VkImage* p) const; };
struct VkImageViewDeleter { void operator()(VkImageView* p) const; };
struct VkBufferDeleter { void operator()(VkBuffer* p) const; };
struct VkSamplerDeleter { void operator()(VkSampler* p) const; };
struct VkFramebufferDeleter { void operator()(VkFramebuffer* p) const; };
struct VkRenderPassDeleter { void operator()(VkRenderPass* p) const; };
struct VkPipelineDeleter { void operator()(VkPipeline* p) const; };
struct VkPipelineLayoutDeleter { void operator()(VkPipelineLayout* p) const; };
struct VkDescriptorSetLayoutDeleter { void operator()(VkDescriptorSetLayout* p) const; };
struct VkShaderModuleDeleter { void operator()(VkShaderModule* p) const; };
struct VkCommandPoolDeleter { void operator()(VkCommandPool* p) const; };
struct VkQueryPoolDeleter { void operator()(VkQueryPool* p) const; };
struct VkSemaphoreDeleter { void operator()(VkSemaphore* p) const; };
struct VkFenceDeleter { void operator()(VkFence* p) const; };
struct VkSwapchainKHRDeleter { void operator()(VkSwapchainKHR* p) const; };
struct VkSurfaceKHRDeleter { void operator()(VkSurfaceKHR* p) const; };
struct VkDescriptorPoolDeleter { void operator()(VkDescriptorPool* p) const; };

using VkInstancePtr = std::shared_ptr<VkInstance>;
using VkDevicePtr = std::shared_ptr<VkDevice>;
using VkImagePtr = std::shared_ptr<VkImage>;
using VkImageViewPtr = std::shared_ptr<VkImageView>;
using VkBufferPtr = std::shared_ptr<VkBuffer>;
using VkSamplerPtr = std::shared_ptr<VkSampler>;
using VkFramebufferPtr = std::shared_ptr<VkFramebuffer>;
using VkRenderPassPtr = std::shared_ptr<VkRenderPass>;
using VkPipelinePtr = std::shared_ptr<VkPipeline>;
using VkPipelineLayoutPtr = std::shared_ptr<VkPipelineLayout>;
using VkDescriptorSetLayoutPtr = std::shared_ptr<VkDescriptorSetLayout>;
using VkShaderModulePtr = std::shared_ptr<VkShaderModule>;
using VkCommandPoolPtr = std::shared_ptr<VkCommandPool>;
using VkQueryPoolPtr = std::shared_ptr<VkQueryPool>;
using VkSemaphorePtr = std::shared_ptr<VkSemaphore>;
using VkFencePtr = std::shared_ptr<VkFence>;
using VkSwapchainKHRPtr = std::shared_ptr<VkSwapchainKHR>;
using VkSurfaceKHRPtr = std::shared_ptr<VkSurfaceKHR>;
using VkDescriptorPoolPtr = std::shared_ptr<VkDescriptorPool>;

SEEK_NAMESPACE_END

