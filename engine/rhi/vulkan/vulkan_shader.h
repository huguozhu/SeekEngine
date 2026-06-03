#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>
#include <vector>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkShader 鈥?Vulkan 鐫€鑹插櫒锛圚LSL 鈫?SPIR-V 缂栬瘧锛?// ============================================================================
class VkShader : public RHIShader
{
public:
    VkShader(Context* context, ShaderType type, std::string const& name,
        std::string const& entry_func_name, std::string const& code);
    ~VkShader() override;

    SResult OnCompile() override;

    VkShaderModule GetVkShaderModule() const { return m_vkShaderModule; }
    VkShaderStageFlagBits GetVkStage() const { return m_vkStage; }
    const std::vector<uint32_t>& GetSPIRVCode() const { return m_SPIRV; }

    // 鍙嶅皠淇℃伅
    struct DescriptorBinding
    {
        uint32_t                binding;
        VkDescriptorType        descriptorType;
        uint32_t                count;
        VkShaderStageFlags      stageFlags;
        std::string             name;
    };
    const std::vector<DescriptorBinding>& GetDescriptorBindings() const { return m_DescriptorBindings; }

private:
    VkShaderModule          m_vkShaderModule = VK_NULL_HANDLE;
    VkShaderStageFlagBits   m_vkStage;
    std::vector<uint32_t>   m_SPIRV;
    std::vector<DescriptorBinding> m_DescriptorBindings;
};

using VkShaderPtr = std::shared_ptr<VkShader>;

SEEK_NAMESPACE_END


