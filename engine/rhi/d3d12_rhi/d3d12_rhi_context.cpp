#include "rhi/d3d12_rhi/d3d12_rhi_context.h"

SEEK_NAMESPACE_BEGIN

D3D12RHIContext::D3D12RHIContext(Context* context)
    :RHIContext(context)
{

}

SResult D3D12RHIContext::Init()
{

    return S_Success;
}
SResult D3D12RHIContext::CheckCapabilitySetSupport()
{
    return S_Success;
}
void D3D12RHIContext::Uninit()
{

}
SResult D3D12RHIContext::AttachNativeWindow(std::string const& name, void* native_wnd)
{
    return S_Success;
}




extern "C"
{
    void MakeD3D12RHIContext(Context* context, RHIContextPtrUnique& out)
    {
        out = MakeUniquePtr<D3D12RHIContext>(context);
    }
}
SEEK_NAMESPACE_END