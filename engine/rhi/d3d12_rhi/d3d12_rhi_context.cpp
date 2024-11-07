#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"

#include "kernel/context.h"

#include "utils/dll_loader.h"

#define SEEK_MACRO_FILE_UID 68     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN


static DllLoader __dxgiDllLoader("dxgi.dll");
static decltype(&::CreateDXGIFactory1) CreateDXGIFactory1 = nullptr;
static decltype(&::CreateDXGIFactory2) CreateDXGIFactory2 = nullptr;

static DllLoader __d3d12DllLoader("d3d12.dll");


static decltype(&::DXGIGetDebugInterface1) DXGIGetDebugInterface1 = nullptr;
static decltype(&::D3D12GetDebugInterface) D3D12GetDebugInterface = nullptr;


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
    //UINT dxgiFactoryFlags = 0;

    //// Enable the debug layer (requires the Graphics Tools "optional feature").
    //// NOTE: Enabling the debug layer after device creation will invalidate the active device.
    //if (m_pContext->GetRenderInitInfo().debug)
    //{
    //    ID3D12DebugPtr debugController;
    //    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    //    {
    //        debugController->EnableDebugLayer();
    //        // Enable additional debug layers.
    //        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    //    }
    //}

    do {
        if (!__dxgiDllLoader.Load())
        {
            LOG_ERROR("load %s fail", __dxgiDllLoader.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }

        if (!__d3d12DllLoader.Load())
        {
            LOG_ERROR("load %s fail", __d3d12DllLoader.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }

        if (!CreateDXGIFactory2)
        {
            CreateDXGIFactory2 = (decltype(CreateDXGIFactory2))__dxgiDllLoader.FindSymbol("CreateDXGIFactory2");
        }

        if (!CreateDXGIFactory2 && !CreateDXGIFactory1)
        {
            CreateDXGIFactory1 = (decltype(CreateDXGIFactory1))__dxgiDllLoader.FindSymbol("CreateDXGIFactory1");
            if (!CreateDXGIFactory1)
            {
                LOG_ERROR("no CreateDXGIFactory1 entry point");
                return ERR_NOT_SUPPORT;
            }
        }




        if (!DXGIGetDebugInterface1)
        {
            DXGIGetDebugInterface1 = (decltype(DXGIGetDebugInterface1))__dxgiDllLoader.FindSymbol("DXGIGetDebugInterface1");
            if (!DXGIGetDebugInterface1)
            {
                LOG_WARNING("no DXGIGetDebugInterface1 entry point");
            }
        }

        if (!D3D12GetDebugInterface)
        {
            D3D12GetDebugInterface = (decltype(D3D12GetDebugInterface))__dxgiDllLoader.FindSymbol("D3D12GetDebugInterface");
        }


    } while (0);
















    //ComPtr<IDXGIFactory4> factory;
    //HRESULT res = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    //if (m_useWarpDevice)
    //{
    //    ComPtr<IDXGIAdapter> warpAdapter;
    //    ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

    //    ThrowIfFailed(D3D12CreateDevice(
    //        warpAdapter.Get(),
    //        D3D_FEATURE_LEVEL_11_0,
    //        IID_PPV_ARGS(&m_device)
    //    ));
    //}
    //else
    //{
    //    ComPtr<IDXGIAdapter1> hardwareAdapter;
    //    GetHardwareAdapter(factory.Get(), &hardwareAdapter);

    //    ThrowIfFailed(D3D12CreateDevice(
    //        hardwareAdapter.Get(),
    //        D3D_FEATURE_LEVEL_11_0,
    //        IID_PPV_ARGS(&m_device)
    //    ));
    //}

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