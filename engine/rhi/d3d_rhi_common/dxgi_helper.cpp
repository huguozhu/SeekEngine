#include "rhi/d3d_rhi_common/dxgi_helper.h"
#include "utils/dll_loader.h"
#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 8     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static const UINT VENDOR_INTEL = 0x8086;
static const UINT VENDOR_AMD = 0x1002;
static const UINT VENDOR_NVIDIA = 0x10de;
static const UINT VENDOR_MICROSOFT = 0x1414;

static DllLoader s_dxgi("dxgi.dll");
static decltype(&::CreateDXGIFactory1) Func_CreateDXGIFactory1 = nullptr;
static decltype(&::CreateDXGIFactory2) Func_CreateDXGIFactory2 = nullptr;
static decltype(&::DXGIGetDebugInterface1) Func_DXGIGetDebugInterface1 = nullptr;

static DllLoader s_dxgi_debug("dxgidebug.dll");
static decltype(&::DXGIGetDebugInterface) Func_DXGIGetDebugInterface = nullptr;
static ComPtr<IDXGIInfoQueue> s_dxgiInfoQueue = nullptr;


SResult DxgiHelper::Init(int32_t preferred_adapter, bool debug)
{
    do {
        if (!s_dxgi.Load())
        {
            LOG_ERROR("load %s fail", s_dxgi.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }

        if (!Func_CreateDXGIFactory2)
        {
            Func_CreateDXGIFactory2 = (decltype(Func_CreateDXGIFactory2))s_dxgi.FindSymbol("CreateDXGIFactory2");
        }

        if (!Func_CreateDXGIFactory2 && !Func_CreateDXGIFactory1)
        {
            Func_CreateDXGIFactory1 = (decltype(Func_CreateDXGIFactory1))s_dxgi.FindSymbol("CreateDXGIFactory1");
            if (!Func_CreateDXGIFactory1)
            {
                LOG_ERROR("Function CreateDXGIFactory1 not found");
                return ERR_NOT_SUPPORT;
            }
        }

        if (!Func_DXGIGetDebugInterface1)
        {
            Func_DXGIGetDebugInterface1 = (decltype(Func_DXGIGetDebugInterface1))s_dxgi.FindSymbol("DXGIGetDebugInterface1");
            if (!Func_DXGIGetDebugInterface1)
            {
                LOG_WARNING("no DXGIGetDebugInterface1 entry point");
            }
        }

        if (Func_DXGIGetDebugInterface1 && !m_pGraphicsAnalysis)
        {
            HRESULT hr = Func_DXGIGetDebugInterface1(0, __uuidof(IDXGraphicsAnalysis), (void**)m_pGraphicsAnalysis.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("DXGIGetDebugInterface1 fail, %x", hr);
            }
        }

        HRESULT hr = S_OK;
        if (Func_CreateDXGIFactory2)
        {
            UINT dxgi_factory_flag = 0;
            if (debug)
                dxgi_factory_flag |= DXGI_CREATE_FACTORY_DEBUG;

            hr = Func_CreateDXGIFactory2(dxgi_factory_flag, __uuidof(IDXGIFactory1), (void**)m_pDxgiFactory.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("CreateDXGIFactory2 Error, hr=%x, try CreateDXGIFactory1", hr);
            }
        }

        if (!m_pDxgiFactory)
        {
            hr = Func_CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)m_pDxgiFactory.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("CreateDXGIFactory1 Error, hr=%x, ", hr);
                break;
            }
        }

        if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory1)))
        {
            m_iDxgiSubVer = 1;
            if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory2)))
            {
                m_iDxgiSubVer = 2;
                if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory3)))
                {
                    m_iDxgiSubVer = 3;
                    if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory4)))
                    {
                        m_iDxgiSubVer = 4;
                        if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory5)))
                        {
                            m_iDxgiSubVer = 5;
                            if (SUCCEEDED(m_pDxgiFactory.As(&m_pDxgiFactory6)))
                            {
                                m_iDxgiSubVer = 6;
                            }
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

        //int32_t preferred_adapter = m_pContext->GetPreferredAdapter();
        if (m_iCurAdapterNo != preferred_adapter && preferred_adapter < m_vAdapterList.size()) {
            m_iCurAdapterNo = preferred_adapter;
            LOG_INFO("user set preferred adapter %d", preferred_adapter);
        }
        else if (preferred_adapter >= m_vAdapterList.size()) {
            LOG_INFO("invalid preferred adapter %d, out of range [0, %d], use default adapter", preferred_adapter, m_vAdapterList.size() - 1);
        }

        LOG_INFO("using adapter: %ls", m_vAdapterList[m_iCurAdapterNo]->DXGIAdapterDesc().Description);

    } while (0);

    return S_Success;
}
void DxgiHelper::Uninit()
{
    m_vAdapterList.clear();
    m_iCurAdapterNo = INVALID_ADAPTER_INDEX;
    m_iDxgiSubVer = 0;
    m_pDxgiFactory.Reset();
    m_pDxgiFactory1.Reset();
    m_pDxgiFactory2.Reset();
    m_pDxgiFactory3.Reset();
    m_pDxgiFactory4.Reset();
    m_pDxgiFactory5.Reset();
    m_pDxgiFactory6.Reset();
}
D3DAdapterPtr DxgiHelper::ActiveAdapter()
{
    if (m_iCurAdapterNo != INVALID_ADAPTER_INDEX)
        return m_vAdapterList[m_iCurAdapterNo];
    else
        return nullptr;
}

void OutputD3DCommonDebugInfo()
{
    if (!Func_DXGIGetDebugInterface)
    {
        if (!s_dxgi_debug.Load())
        {
            LOG_ERROR("no %s.dll", s_dxgi_debug.dllname.c_str());
            return;
        }
        Func_DXGIGetDebugInterface = (decltype(Func_DXGIGetDebugInterface))s_dxgi_debug.FindSymbol("DXGIGetDebugInterface");
        if (!Func_DXGIGetDebugInterface)
        {
            LOG_ERROR("Function DXGIGetDebugInterface not found.");
            return;
        }
    }

    if (!s_dxgiInfoQueue)
    {
        HRESULT hr = Func_DXGIGetDebugInterface(__uuidof(IDXGIInfoQueue), (void**)s_dxgiInfoQueue.GetAddressOf());
        if (FAILED(hr))
        {
            LOG_ERROR("DXGIGetDebugInterface Error, hr=%x", hr);
            return;
        }
    }

    ComPtr<IDXGIDebug> dxgiDebug;
    if (SUCCEEDED(s_dxgiInfoQueue.As(&dxgiDebug)))
    {
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
    dxgiDebug.Reset();

    UINT64 message_count = s_dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    for (UINT64 i = 0; i < message_count; i++)
    {
        SIZE_T messageLength = 0;
        if (s_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength) == S_FALSE)
        {
            DXGI_INFO_QUEUE_MESSAGE* pMessage = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
            if (pMessage && s_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength) == S_OK)
            {
                LOG_INFO("DXGI Debug: %s", pMessage->pDescription);
            }
            if (pMessage)
            {
                free(pMessage);
            }
        }
    }
    s_dxgiInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
}

SEEK_NAMESPACE_END
