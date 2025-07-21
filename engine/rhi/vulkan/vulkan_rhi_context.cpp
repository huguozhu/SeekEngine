#include "rhi/vulkan_rhi/vulkan_rhi_context.h"

SEEK_NAMESPACE_BEGIN

VulkanRHIContext::VulkanRHIContext(Context* context)
    :RHIContext(context)
{
}
SResult VulkanRHIContext::Init()
{

    return S_Success;
}
SResult VulkanRHIContext::CheckCapabilitySetSupport()
{
    return S_Success;
}
void VulkanRHIContext::Uninit()
{

}
SResult VulkanRHIContext::AttachNativeWindow(std::string const& name, void* native_wnd)
{
    return S_Success;
}



extern "C"
{
    void MakeVulkanRHIContext(Context* context, RHIContextPtrUnique& out)
    {
        out = MakeUniquePtr<VulkanRHIContext>(context);
    }
}

SEEK_NAMESPACE_END