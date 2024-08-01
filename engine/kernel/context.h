#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN


enum class RHIType
{
    Unknown,
    D3D12,
    Vulkan,
};

struct RenderInitInfo
{
    bool                    debug = false;
    bool                    profile = false;
    RHIType                 rhi_type = RHIType::Unknown;
    uint32_t                num_samples = 1;
    int32_t                 preferred_adapter = -1;
    void*                   native_wnd = nullptr;
    void*                   device = nullptr;
};

class Context
{
public:
    Context();
    ~Context();


};

SEEK_NAMESPACE_END