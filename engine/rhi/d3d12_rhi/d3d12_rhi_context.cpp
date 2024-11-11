#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"

#include "kernel/context.h"

#include "utils/dll_loader.h"

#define SEEK_MACRO_FILE_UID 68     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static const UINT VENDOR_INTEL = 0x8086;
static const UINT VENDOR_AMD = 0x1002;
static const UINT VENDOR_NVIDIA = 0x10de;
static const UINT VENDOR_MICROSOFT = 0x1414;

static DllLoader __dxgi12("dxgi.dll");
static decltype(&::CreateDXGIFactory2)      FUNC_CreateDXGIFactory2 = nullptr;
static decltype(&::DXGIGetDebugInterface1)  FUNC_DXGIGetDebugInterface1 = nullptr;

static DllLoader __d3d12("d3d12.dll");
static decltype(&::D3D12GetDebugInterface) FUNC_D3D12GetDebugInterface = nullptr;

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
    do {
        if (!__dxgi12.Load())
        {
            LOG_ERROR("load %s fail", __dxgi12.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }
        if (!__d3d12.Load())
        {
            LOG_ERROR("load %s fail", __d3d12.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }
        if (!FUNC_CreateDXGIFactory2)
        {
            FUNC_CreateDXGIFactory2 = (decltype(FUNC_CreateDXGIFactory2))__dxgi12.FindSymbol("CreateDXGIFactory2");
        }

        if (!FUNC_DXGIGetDebugInterface1)
        {
            FUNC_DXGIGetDebugInterface1 = (decltype(FUNC_DXGIGetDebugInterface1))__dxgi12.FindSymbol("DXGIGetDebugInterface1");
            if (!FUNC_DXGIGetDebugInterface1)
                LOG_WARNING("Function DXGIGetDebugInterface1 not found.");
        }

        if (FUNC_DXGIGetDebugInterface1 && !m_pGraphicsAnalysis)
        {
            HRESULT hr = FUNC_DXGIGetDebugInterface1(0, __uuidof(IDXGraphicsAnalysis), (void**)m_pGraphicsAnalysis.GetAddressOf());
            if (FAILED(hr))
                LOG_WARNING("DXGIGetDebugInterface1 fail, %x", hr);
        }

        if (!FUNC_D3D12GetDebugInterface)
        {
            FUNC_D3D12GetDebugInterface = (decltype(FUNC_D3D12GetDebugInterface))__d3d12.FindSymbol("D3D12GetDebugInterface");
            if (!FUNC_D3D12GetDebugInterface)
                LOG_WARNING("Function D3D12GetDebugInterface not found");
        }


        HRESULT hr = S_OK;
        if (FUNC_CreateDXGIFactory2)
        {
            UINT dxgiFactoryFlags = 0;
            if (m_pContext->IsDebug())
            {
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
            hr = FUNC_CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_pDxgiFactory1));
            if (FAILED(hr))
            {
                LOG_ERROR("D3D12RHIContext::Init() CreateDXGIFactory2 error");
            }
        }

        m_iDxgiSubVer = 1;
        if (SUCCEEDED(m_pDxgiFactory1.As(&m_pDxgiFactory2)))
        {
            m_iDxgiSubVer = 2;
            if (SUCCEEDED(m_pDxgiFactory1.As(&m_pDxgiFactory3)))
            {
                m_iDxgiSubVer = 3;
                if (SUCCEEDED(m_pDxgiFactory1.As(&m_pDxgiFactory4)))
                {
                    m_iDxgiSubVer = 4;
                    if (SUCCEEDED(m_pDxgiFactory1.As(&m_pDxgiFactory5)))
                    {
                        m_iDxgiSubVer = 5;
                        if (SUCCEEDED(m_pDxgiFactory1.As(&m_pDxgiFactory6)))
                        {
                            m_iDxgiSubVer = 6;
                        }
                    }
                }
            }
        }
        LOG_INFO("dxgi runtime version: 1.%d", m_iDxgiSubVer);

        auto EnumAdaptersProc = [this](UINT adapter_no, IDXGIAdapter1** dxgi_adapter, bool useDxgiFactory1) -> HRESULT
            {
                if (m_pDxgiFactory6 && !useDxgiFactory1)
                {
                    return m_pDxgiFactory6->EnumAdapterByGpuPreference(adapter_no, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter1), (void**)dxgi_adapter);
                }
                else
                {
                    // EnumAdapters1 first returns the adapter with the output on which the desktop primary is displayed.
                    return m_pDxgiFactory1->EnumAdapters1(adapter_no, dxgi_adapter);
                }
            };

        bool preferToUseDxgiFactory1 = !m_pDxgiFactory6;
        UINT adapter_no = 0;
        do {
            IDXGIAdapter1Ptr dxgi_adapter = nullptr;
            while (SUCCEEDED(EnumAdaptersProc(adapter_no, dxgi_adapter.ReleaseAndGetAddressOf(), preferToUseDxgiFactory1)))
            {
                m_vAdapterList.push_back(MakeSharedPtr<D3DAdapter>(adapter_no++, dxgi_adapter.Get()));
            }

            if (!m_vAdapterList.empty() || preferToUseDxgiFactory1)
                break;

            LOG_INFO("use IDXGIFactory6::EnumAdapterByGpuPreference enum adapters fail, try IDXGIFactory1");
            preferToUseDxgiFactory1 = true;
        } while (1);

        LOG_INFO("available adapters:");
        for (size_t i = 0; i != m_vAdapterList.size(); i++)
        {
            LOG_INFO("  %2d: %ls", i, m_vAdapterList[i]->DXGIAdapterDesc().Description);
            LOG_INFO("    - VendorID: 0x%04x", m_vAdapterList[i]->DXGIAdapterDesc().VendorId);
            LOG_INFO("    - DeviceId: 0x%04x", m_vAdapterList[i]->DXGIAdapterDesc().DeviceId);
            LOG_INFO("    - SubSysId: 0x%04x", m_vAdapterList[i]->DXGIAdapterDesc().SubSysId);
            LOG_INFO("    - Revision: 0x%04x", m_vAdapterList[i]->DXGIAdapterDesc().Revision);
            LOG_INFO("    - AdapterLuid: %lu %lu", m_vAdapterList[i]->DXGIAdapterDesc().AdapterLuid.HighPart, m_vAdapterList[i]->DXGIAdapterDesc().AdapterLuid.LowPart);
        }

        if (adapter_no > 0)
            m_iCurAdapterNo = 0;

        if (m_iCurAdapterNo == INVALID_ADAPTER_INDEX)
        {
            LOG_ERROR("no suitable adapter");
            break;
        }

        // prefer to use Intel&AMD than Nvidia
        if (m_vAdapterList[m_iCurAdapterNo]->DXGIAdapterDesc().VendorId == VENDOR_NVIDIA)
        {
            for (size_t i = 0; i != m_vAdapterList.size(); i++)
            {
                if (i == m_iCurAdapterNo)
                    continue;

                if (m_vAdapterList[i]->DXGIAdapterDesc().VendorId == VENDOR_INTEL ||
                    m_vAdapterList[i]->DXGIAdapterDesc().VendorId == VENDOR_AMD)
                {
                    m_iCurAdapterNo = i;
                    break;
                }
            }
        }

        int32_t preferred_adapter = m_pContext->GetPreferredAdapter();
        if (m_iCurAdapterNo != preferred_adapter && preferred_adapter < m_vAdapterList.size()) {
            m_iCurAdapterNo = preferred_adapter;
            LOG_INFO("user set preferred adapter %d", preferred_adapter);
        }
        else if (preferred_adapter >= m_vAdapterList.size()) {
            LOG_INFO("invalid preferred adapter %d, out of range [0, %d], use default adapter", preferred_adapter, m_vAdapterList.size() - 1);
        }

        LOG_INFO("using adapter: %ls", m_vAdapterList[m_iCurAdapterNo]->DXGIAdapterDesc().Description);

        UINT flags = 0;
        if (m_pContext->IsDebug())
        {
            //flags |= D3D12_CREATE_DEVICE_DEBUG;
        }
        D3D_FEATURE_LEVEL feature_levels[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        uint32_t feature_level_start_index = 0;
        if (m_iDxgiSubVer < 4)
        {
            feature_level_start_index = 2;
            if (m_iDxgiSubVer < 2)
            {
                feature_level_start_index = 3;
            }
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