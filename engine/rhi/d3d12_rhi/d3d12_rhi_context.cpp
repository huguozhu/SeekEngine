#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"

#include "kernel/context.h"

#include "utils/dll_loader.h"

#define SEEK_MACRO_FILE_UID 68     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static DllLoader s_d3d12("d3d12.dll");
static decltype(&::D3D12GetDebugInterface) FUNC_D3D12GetDebugInterface = nullptr;
static decltype(&::D3D12CreateDevice) FUNC_D3D12CreateDevice = nullptr;

const char* GetD3D12FeatureLevelStr(D3D_FEATURE_LEVEL feature_level)
{
    switch (feature_level)
    {
    case D3D_FEATURE_LEVEL_12_2:
        return "12.2";
    case D3D_FEATURE_LEVEL_12_1:
        return "12.1";
    case D3D_FEATURE_LEVEL_12_0:
        return "12.0";
    default:
        return "unknown";
    }
}

D3D12RHIContext::D3D12RHIContext(Context* context)
    :RHIContext(context)
{
}

SResult D3D12RHIContext::Init()
{
    SEEK_RETIF_FAIL(DxgiHelper::Init(m_pContext->GetPreferredAdapter(), m_pContext->EnableDebug()));
    do {
        HRESULT hr = S_OK;
        if (!s_d3d12.Load())
        {
            LOG_ERROR("load %s fail", s_d3d12.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }              

        if (!FUNC_D3D12CreateDevice)
        {
            FUNC_D3D12CreateDevice = (decltype(FUNC_D3D12CreateDevice))s_d3d12.FindSymbol("D3D12CreateDevice");
            if (!FUNC_D3D12CreateDevice)
            {
                LOG_ERROR("Function D3D12CreateDevice not found.");
                return ERR_NOT_SUPPORT;
            }
        }

        if (m_pContext->EnableDebug())
        {
            if (!FUNC_D3D12GetDebugInterface)
            {
                FUNC_D3D12GetDebugInterface = (decltype(FUNC_D3D12GetDebugInterface))s_d3d12.FindSymbol("D3D12GetDebugInterface");
                if (!FUNC_D3D12GetDebugInterface)
                {
                    LOG_ERROR("Function D3D12GetDebugInterface not found.");
                    return ERR_NOT_SUPPORT;
                }
            }

            if (SUCCEEDED(FUNC_D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)m_pDebugCtrl.GetAddressOf())))
                m_pDebugCtrl->EnableDebugLayer();
        }

        hr = FUNC_D3D12CreateDevice(m_vAdapterList[m_iCurAdapterNo]->DXGIAdapter(),
                D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device),
                (void**)m_pDevice.GetAddressOf() );
        if (FAILED(hr))
        {
            LOG_ERROR("D3D12CreateDevice error.");
            return ERR_NOT_SUPPORT;
        }
    } while (0);

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