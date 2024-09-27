#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_adapter.h"
#include "rhi/d3d11_rhi/d3d11_window.h"
#include "rhi/d3d11_rhi/d3d11_program.h"
#include "rhi/d3d11_rhi/d3d11_mesh.h"
#include "rhi/d3d11_rhi/d3d11_render_buffer.h"
#include "rhi/d3d11_rhi/d3d11_render_state.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "rhi/d3d11_rhi/d3d11_translate.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"

#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_program.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 6     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static const UINT VENDOR_INTEL = 0x8086;
static const UINT VENDOR_AMD = 0x1002;
static const UINT VENDOR_NVIDIA = 0x10de;
static const UINT VENDOR_MICROSOFT = 0x1414;

static DllLoader __dxgiDebugDllLoader("dxgidebug.dll");
static decltype(&::DXGIGetDebugInterface) DXGIGetDebugInterface = nullptr;
static ComPtr<IDXGIInfoQueue> __dxgiInfoQueue = nullptr;

static DllLoader __dxgiDllLoader("dxgi.dll");
static decltype(&::CreateDXGIFactory1) CreateDXGIFactory1 = nullptr;
static decltype(&::CreateDXGIFactory2) CreateDXGIFactory2 = nullptr;
static decltype(&::DXGIGetDebugInterface1) DXGIGetDebugInterface1 = nullptr;

static DllLoader __d3d11DllLoader("d3d11.dll");
static decltype(&::D3D11CreateDevice) D3D11CreateDevice = nullptr;

const char* GetD3DFeatureLevelStr(D3D_FEATURE_LEVEL feature_level)
{
    switch (feature_level)
    {
        case D3D_FEATURE_LEVEL_12_1:
            return "12.1";
        case D3D_FEATURE_LEVEL_12_0:
            return "12.0";
        case D3D_FEATURE_LEVEL_11_1:
            return "11.1";
        case D3D_FEATURE_LEVEL_11_0:
            return "11.0";
        default:
            return "unknown";
    }
}

D3D11RHIContext::D3D11RHIContext(Context* context)
    : RHIContext(context)
    , m_iCurAdapterNo(INVALID_ADAPTER_INDEX)
    , m_pDevice(nullptr)
    , m_pDxgiFactory1(nullptr)
    , m_pDeviceContext(nullptr)
{
    
}

D3D11RHIContext::~D3D11RHIContext()
{
    Uninit();
}

SResult D3D11RHIContext::Init()
{
    do {
        if (!__dxgiDllLoader.Load())
        {
            LOG_ERROR("load %s fail", __dxgiDllLoader.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }

        if (!__d3d11DllLoader.Load())
        {
            LOG_ERROR("load %s fail", __d3d11DllLoader.dllname.c_str());
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

        if (DXGIGetDebugInterface1 && !m_pGraphicsAnalysis)
        {
            HRESULT hr = DXGIGetDebugInterface1(0, __uuidof(IDXGraphicsAnalysis), (void**)m_pGraphicsAnalysis.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("DXGIGetDebugInterface1 fail, %x", hr);
            }
        }

        if (!D3D11CreateDevice)
        {
            D3D11CreateDevice = (decltype(D3D11CreateDevice))__d3d11DllLoader.FindSymbol("D3D11CreateDevice");
            if (!D3D11CreateDevice)
            {
                LOG_ERROR("no D3D11CreateDevice entry point");
                return ERR_NOT_SUPPORT;
            }
        }

        HRESULT hr = S_OK;
        if (CreateDXGIFactory2)
        {
            UINT dxgi_factory_flag = 0;
            if (m_pContext->IsDebug())
            {
                dxgi_factory_flag |= DXGI_CREATE_FACTORY_DEBUG;
            }
            hr = CreateDXGIFactory2(dxgi_factory_flag, __uuidof(IDXGIFactory1), (void**)m_pDxgiFactory1.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("CreateDXGIFactory2 Error, hr=%x, try CreateDXGIFactory1", hr);
                // no break, try CreateDXGIFactory1
            }
        }

        if (!m_pDxgiFactory1)
        {
            hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)m_pDxgiFactory1.GetAddressOf());
            if (FAILED(hr))
            {
                LOG_WARNING("CreateDXGIFactory1 Error, hr=%x, ", hr);
                break;
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
                m_vAdapterList.push_back(MakeSharedPtr<D3D11Adapter>(adapter_no++, dxgi_adapter.Get()));
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
        else if (preferred_adapter >= m_vAdapterList.size()){
            LOG_INFO("invalid preferred adapter %d, out of range [0, %d], use default adapter", preferred_adapter, m_vAdapterList.size() - 1);
        }

        LOG_INFO("using adapter: %ls", m_vAdapterList[m_iCurAdapterNo]->DXGIAdapterDesc().Description);

        UINT flags = 0;
        if (m_pContext->IsDebug())
        {
            flags |= D3D11_CREATE_DEVICE_DEBUG;
        }
        D3D_FEATURE_LEVEL feature_levels[] =
        {
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

        D3D_FEATURE_LEVEL out_feature_level;
        uint32_t feature_level_count = sizeof(feature_levels) / sizeof(D3D_FEATURE_LEVEL);
        while (feature_level_start_index < feature_level_count)
        {
            hr = D3D11CreateDevice(m_vAdapterList[m_iCurAdapterNo]->DXGIAdapter(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                flags,
                &feature_levels[feature_level_start_index],
                feature_level_count - feature_level_start_index,
                D3D11_SDK_VERSION,
                m_pDevice.ReleaseAndGetAddressOf(),
                &out_feature_level,
                m_pDeviceContext.ReleaseAndGetAddressOf());
            if (SUCCEEDED(hr))
                break;

            if (FAILED(hr) && 0 != (flags & D3D11_CREATE_DEVICE_DEBUG))
            {
                // Try without debug in case D3D11 SDK Layers is not present?
                flags &= ~D3D11_CREATE_DEVICE_DEBUG;
                continue;
            }
            if (m_pContext->IsDebug())
            {
                flags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            feature_level_start_index++;
        }

        if (FAILED(hr))
        {
            LOG_ERROR("D3D11CreateDevice Error, hr:%x", hr);
            break;
        }
        LOG_INFO("device supported feature level %s", GetD3DFeatureLevelStr(out_feature_level));

        //if (auto dxgi_device = pDevice.try_as<IDXGIDevice2>())
        //ComPtr<IDXGIDevice1> dxgi_device = nullptr;
        //if (SUCCEEDED(m_pDevice.As(&dxgi_device)))
        //{
        //    ComPtr<IDXGIAdapter> adapter;
        //    dxgi_device->GetAdapter(adapter.GetAddressOf());
        //    ComPtr<IDXGIAdapter1> adapter1;
        //    adapter.As(&adapter1);
        //    m_vAdapterList[m_iCurAdapterNo]->ResetAdapter(adapter1.Get());
        //}

        if (m_pContext->IsDebug())
        {
            ComPtr<ID3D11Debug> d3dDebug = nullptr;
            if (SUCCEEDED(m_pDevice.As(&d3dDebug)))
            {
                ComPtr<ID3D11InfoQueue> d3dInfoQueue = nullptr;
                if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
                {
                    // break when corruption and error occurs
                    d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                    d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

                    //D3D11_MESSAGE_ID hide[] =
                    //{
                    //    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                    //};

                    //D3D11_INFO_QUEUE_FILTER filter = {};
                    //filter.DenyList.NumIDs = _countof(hide);
                    //filter.DenyList.pIDList = hide;
                    //d3dInfoQueue->AddStorageFilterEntries(&filter);
                }
            }
        }

        return CheckCapabilitySetSupport();
    } while (0);

    Uninit();
    return ERR_NOT_SUPPORT;
}

void D3D11RHIContext::Uninit()
{
    if (m_pDeviceContext)
    {
        m_pDeviceContext->ClearState();
        m_pDeviceContext->Flush();
    }
    m_pDeviceContext.Reset();
    m_pDevice.Reset();
    m_vAdapterList.clear();
    m_iCurAdapterNo = INVALID_ADAPTER_INDEX;
    m_iDxgiSubVer = 0;
    m_pDxgiFactory1.Reset();
    m_pDxgiFactory2.Reset();
    m_pDxgiFactory3.Reset();
    m_pDxgiFactory4.Reset();
    m_pDxgiFactory5.Reset();
    m_pDxgiFactory6.Reset();
}

SResult D3D11RHIContext::CheckCapabilitySetSupport()
{
    seek_memset_s(m_CapabilitySet.TextureSampleCountSupport, sizeof(m_CapabilitySet.TextureSampleCountSupport), false, sizeof(m_CapabilitySet.TextureSampleCountSupport));
    static_assert((1 << 4) == CAP_MAX_TEXTURE_SAMPLE_COUNT, "msaa sample count mismatch");
    for (size_t msaaLevel = 0; msaaLevel <= 4; msaaLevel++)
    {
        UINT sampleCount = 1 << msaaLevel;
        UINT qualityLevel;
        HRESULT hr = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM/*FIXME: not hardcode*/, sampleCount, &qualityLevel);
        if (SUCCEEDED(hr) && qualityLevel > 0)
        {
            m_msaa[sampleCount].Count = sampleCount;
            m_msaa[sampleCount].Quality = qualityLevel - 1;
            m_CapabilitySet.TextureSampleCountSupport[sampleCount] = true;
        }
    }

    std::string supportedMSAA = "{ ";
    for (size_t i = 1; i < ARRAYSIZE(m_CapabilitySet.TextureSampleCountSupport); i++)
    {
        if (m_CapabilitySet.TextureSampleCountSupport[i])
            supportedMSAA += std::to_string(i) + " ";
    }
    supportedMSAA += "}";
    LOG_INFO("supported msaa: %s", supportedMSAA.c_str());

    for (size_t i = 0; i < (size_t)PixelFormat::Num; i++)
    {
        DXGI_FORMAT dxgi_format = D3D11Translate::TranslateToPlatformFormat((PixelFormat)i);
        UINT supported_bits;
        HRESULT hr = m_pDevice->CheckFormatSupport(dxgi_format, &supported_bits);
        if (SUCCEEDED(hr))
        {
            if (supported_bits & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE)
                m_CapabilitySet.TextureSupport[i][(uint32_t)TextureFormatSupportType::Filtering] = true;
            if (supported_bits & D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW)
                m_CapabilitySet.TextureSupport[i][(uint32_t)TextureFormatSupportType::Write] = true;
            if (supported_bits & D3D11_FORMAT_SUPPORT_RENDER_TARGET)
                m_CapabilitySet.TextureSupport[i][(uint32_t)TextureFormatSupportType::RenderTarget] = true;
        }
    }

    return S_Success;
}

D3D11AdapterPtr D3D11RHIContext::ActiveAdapter()
{
    if (m_iCurAdapterNo != INVALID_ADAPTER_INDEX)
        return m_vAdapterList[m_iCurAdapterNo];
    else
        return nullptr;
}

void D3D11RHIContext::SetDXGIFactory1(IDXGIFactory1* p)
{
    m_pDxgiFactory1 = p;
}

void D3D11RHIContext::SetD3D11Device(ID3D11Device* p)
{
    m_pDevice = p;
}

void D3D11RHIContext::SetD3D11DeviceContext(ID3D11DeviceContext* p)
{
    m_pDeviceContext = p;
}

SResult D3D11RHIContext::AttachNativeWindow(std::string const& name, void* native_wnd)
{
    SResult res = S_Success;
    D3D11AdapterPtr pAdapter = this->ActiveAdapter();
    if (native_wnd)
    {
        D3D11WindowPtr win = MakeSharedPtr<D3D11Window>(m_pContext);
        res = win->Create(pAdapter.get(), name, native_wnd);
        if (SEEK_CHECKFAILED(res))
            return res;
        this->BindRHIFrameBuffer(win);
        m_pScreenRHIFrameBuffer = m_pCurRHIFrameBuffer;
    }
    return res;
}

void D3D11RHIContext::SetD3DRasterizerState(ID3D11RasterizerState* state)
{
    m_pDeviceContext->RSSetState(state);
}

void D3D11RHIContext::SetD3DDepthStencilState(ID3D11DepthStencilState* state, uint16_t stencil_ref)
{
    m_pDeviceContext->OMSetDepthStencilState(state, stencil_ref);
}

void D3D11RHIContext::SetD3DBlendState(ID3D11BlendState* state, float4 blend_factor, uint32_t sample_mask)
{
    m_pDeviceContext->OMSetBlendState(state, &(blend_factor[0]), sample_mask);
}

void D3D11RHIContext::SetD3DShader(ShaderType type, ID3D11DeviceChild* shader)
{
    switch (type)
    {
    case ShaderType::Vertex:
        m_pDeviceContext->VSSetShader((ID3D11VertexShader*)shader, nullptr, 0);
        break;
    case ShaderType::Pixel:
        m_pDeviceContext->PSSetShader((ID3D11PixelShader*)shader, nullptr, 0);
        break;
    case ShaderType::Geometry:
        m_pDeviceContext->GSSetShader((ID3D11GeometryShader*)shader, nullptr, 0);
        break;
    case ShaderType::Hull:
        m_pDeviceContext->HSSetShader((ID3D11HullShader*)shader, nullptr, 0);
        break;
    case ShaderType::Domain:
        m_pDeviceContext->DSSetShader((ID3D11DomainShader*)shader, nullptr, 0);
        break;
    case ShaderType::Compute:
        m_pDeviceContext->CSSetShader((ID3D11ComputeShader*)shader, nullptr, 0);
        break;
    default:
        break;
    }
}

void D3D11RHIContext::SetD3DShaderResourceViews(ShaderType type, uint32_t start_slot, uint32_t num_srvs, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    switch (type)
    {
    case ShaderType::Vertex:
        m_pDeviceContext->VSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    case ShaderType::Pixel:
        m_pDeviceContext->PSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    case ShaderType::Geometry:
        m_pDeviceContext->GSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    case ShaderType::Hull:
        m_pDeviceContext->HSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    case ShaderType::Domain:
        m_pDeviceContext->DSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    case ShaderType::Compute:
        m_pDeviceContext->CSSetShaderResources(start_slot, num_srvs, ppShaderResourceViews);
        break;
    default:
        break;
    }
}

void D3D11RHIContext::SetD3DSamplers(ShaderType type, uint32_t start_slot, uint32_t num_samplers, ID3D11SamplerState* const* ppSampleers)
{
    switch (type)
    {
    case ShaderType::Vertex:
        m_pDeviceContext->VSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    case ShaderType::Pixel:
        m_pDeviceContext->PSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    case ShaderType::Geometry:
        m_pDeviceContext->GSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    case ShaderType::Hull:
        m_pDeviceContext->HSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    case ShaderType::Domain:
        m_pDeviceContext->DSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    case ShaderType::Compute:
        m_pDeviceContext->CSSetSamplers(start_slot, num_samplers, ppSampleers);
        break;
    default:
        break;
    }
}

void D3D11RHIContext::SetD3DUnorderedAccessViews(ShaderType type, uint32_t start_slot, uint32_t num_uavs, ID3D11UnorderedAccessView* const* ppUAV)
{
    switch (type)
    {
    case ShaderType::Compute:
        m_pDeviceContext->CSSetUnorderedAccessViews(start_slot, num_uavs, ppUAV, nullptr);
        break;
    default:
        break;
    }
}

void D3D11RHIContext::SetD3DConstantBuffers(ShaderType type, uint32_t start_slot, uint32_t num_cbuffers, ID3D11Buffer* const* ppCBuffers)
{
    switch (type)
    {
    case ShaderType::Vertex:
        m_pDeviceContext->VSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    case ShaderType::Pixel:
        m_pDeviceContext->PSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    case ShaderType::Geometry:
        m_pDeviceContext->GSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    case ShaderType::Hull:
        m_pDeviceContext->HSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    case ShaderType::Domain:
        m_pDeviceContext->DSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    case ShaderType::Compute:
        m_pDeviceContext->CSSetConstantBuffers(start_slot, num_cbuffers, ppCBuffers);
        break;
    default:
        break;
    }
}

SResult D3D11RHIContext::BeginFrame()
{
    return S_Success;
}

SResult D3D11RHIContext::EndFrame()
{
    return S_Success;
}

SResult D3D11RHIContext::BeginRenderPass(const RenderPassInfo& renderPassInfo)
{
    if (!renderPassInfo.fb)
        return ERR_INVALID_DATA;

    SResult ret = renderPassInfo.fb->Bind();
    if (SEEK_CHECKFAILED(ret))
    {
        LOG_ERROR("bind RHIFrameBuffer fail, ret:%x", ret);
        return ret;
    }
    m_pCurrentRHIFrameBuffer = static_cast<D3D11RHIFrameBuffer*>(renderPassInfo.fb);

    D3D11_VIEWPORT d3dViewport;
    d3dViewport.TopLeftX = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().left;
    d3dViewport.TopLeftY = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().top;
    d3dViewport.Width = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().width;
    d3dViewport.Height = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().height;
    d3dViewport.MinDepth = 0.0;
    d3dViewport.MaxDepth = 1.0;
    m_pDeviceContext->RSSetViewports(1, &d3dViewport);
    return S_Success;
}

SResult D3D11RHIContext::Render(RHIProgram* program, RHIMeshPtr const& mesh)
{
    if (!m_pCurrentRHIFrameBuffer)
    {
        LOG_ERROR("no RHIFrameBuffer is bound, call Render between BeginRenderPass/EndRenderPass");
        return ERR_INVALID_INVOKE_FLOW;
    }

    SResult ret = S_Success;

    RHIRenderState* rs = mesh->GetRenderState().get();

    SEEK_RETIF_FAIL(((D3D11RenderState*)(rs))->Active());
    SEEK_RETIF_FAIL(((D3D11RHIProgram*)(program))->Active());

    D3D11Mesh& d3d_mesh = static_cast<D3D11Mesh&>(*mesh);
    SEEK_RETIF_FAIL(d3d_mesh.Active(program));

    if (mesh->IsInstanceRendering())
    {
        uint32_t instance_count = mesh->GetInstanceCount();
        if (mesh->IsUseIndices())
        {
            uint32_t index_count = mesh->GetNumIndices();
            m_pDeviceContext->DrawIndexedInstanced(index_count, instance_count, 0, 0, 0);
        }
        else
        {
            uint32_t vertex_count = mesh->GetNumVertex();
            m_pDeviceContext->DrawInstanced(vertex_count, instance_count, 0, 0);
        }
    }
    else
    {
        if (mesh->IsUseIndices())
        {
            uint32_t index_count = mesh->GetNumIndices();
            m_pDeviceContext->DrawIndexed(index_count, 0, 0);
        }
        else
        {
            uint32_t vertex_count = mesh->GetNumVertex();
            m_pDeviceContext->Draw(vertex_count, 0);
        }
    }

    SEEK_RETIF_FAIL(d3d_mesh.Deactive());
    ((D3D11RHIProgram*)program)->Deactive();

    //std::vector<ID3D11ShaderResourceView*> null_srvs(16);
    //m_pDeviceContext->PSSetShaderResources(0, 16, null_srvs.data());
    return ret;
}

SResult D3D11RHIContext::EndRenderPass()
{
    if (m_pCurrentRHIFrameBuffer)
    {
        m_pCurrentRHIFrameBuffer->Unbind();
        m_pCurrentRHIFrameBuffer = nullptr;
    }
    return S_Success;
}

void D3D11RHIContext::BeginComputePass(const ComputePassInfo& computePassInfo)
{

}

SResult D3D11RHIContext::Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z)
{
    SResult res = S_Success;
    SEEK_RETIF_FAIL(((D3D11RHIProgram*)(program))->Active());
    x = x <= D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION ? x : D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
    y = y <= D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION ? y : D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
    z = z <= D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION ? z : D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
    m_pDeviceContext->Dispatch(x, y, z);
    ((D3D11RHIProgram*)program)->Deactive();
    return res;
}
SResult D3D11RHIContext::DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf)
{
    SResult res = S_Success;
    SEEK_RETIF_FAIL(((D3D11RHIProgram*)(program))->Active());
    D3D11RHIRenderBuffer* pD3DBuf = (D3D11RHIRenderBuffer*)indirectBuf.get();
    m_pDeviceContext->DispatchIndirect(pD3DBuf->GetD3DBuffer(), 0);
    ((D3D11RHIProgram*)program)->Deactive();
    return res;
}
SResult D3D11RHIContext::DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type)
{
    SResult res = S_Success;
    SEEK_RETIF_FAIL(((D3D11RenderState*)(rs.get()))->Active());
    SEEK_RETIF_FAIL(((D3D11RHIProgram*)(program))->Active());
    D3D11RHIRenderBuffer* pD3DBuf = (D3D11RHIRenderBuffer*)indirectBuf.get();
    m_pDeviceContext->IASetPrimitiveTopology(D3D11Translate::TranslatePrimitiveTopology(type));
    m_pDeviceContext->DrawInstancedIndirect(pD3DBuf->GetD3DBuffer(), 0);
    ((D3D11RHIProgram*)program)->Deactive();
    return res;
}
void D3D11RHIContext::EndComputePass()
{

}

SResult D3D11RHIContext::CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst)
{
    D3D11Texture* src = dynamic_cast<D3D11Texture*>(tex_src.get());
    D3D11Texture* dst = dynamic_cast<D3D11Texture*>(tex_dst.get());
    m_pDeviceContext->CopyResource(dst->GetD3DTexture(), src->GetD3DTexture());

    return S_Success;
}

void D3D11RHIContext::BeginCapture()
{
    if (m_pGraphicsAnalysis)
    {
        m_pGraphicsAnalysis->BeginCapture();
    }
}

void D3D11RHIContext::EndCapture()
{
    if (m_pGraphicsAnalysis)
    {
        m_pGraphicsAnalysis->EndCapture();
    }
}

void D3D11RHIContext::BeginRHITimerQuery(RHITimerQueryPtr& timerRHIQuery)
{
    m_RHITimerQueryExecutor.Begin(timerRHIQuery);
}

void D3D11RHIContext::EndRHITimerQuery(RHITimerQueryPtr& timerRHIQuery)
{
    m_RHITimerQueryExecutor.End(timerRHIQuery);
}

void D3D11RHIContext::BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name)
{
    ID3D11Buffer* d3d_buffer = cbuffer == nullptr ? nullptr : ((D3D11RHIRenderBuffer*)cbuffer)->GetD3DBuffer();
    SetD3DConstantBuffers(stage, binding, 1, &d3d_buffer);
}

void D3D11RHIContext::BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name)
{
    ID3D11ShaderResourceView* srv = buffer == nullptr ? nullptr : ((D3D11RHIRenderBuffer*)buffer)->GetD3DShaderResourceView();
    SetD3DShaderResourceViews(stage, binding, 1, &srv);
}

void D3D11RHIContext::BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name)
{
    ID3D11UnorderedAccessView* uav = rw_buffer == nullptr ? nullptr : ((D3D11RHIRenderBuffer*)rw_buffer)->GetD3DUnorderedAccessView();
    SetD3DUnorderedAccessViews(stage, binding, 1, &uav);
}

void D3D11RHIContext::BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name)
{
    ID3D11ShaderResourceView* srv = texture == nullptr ? nullptr : ((D3D11Texture*)texture)->GetD3DShaderResourceView();
    SetD3DShaderResourceViews(stage, binding, 1, &srv);
}

void D3D11RHIContext::BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name)
{
    ID3D11UnorderedAccessView* uav = rw_texture == nullptr ? nullptr : ((D3D11Texture*)rw_texture)->GetD3DUnorderedAccessView();
    SetD3DUnorderedAccessViews(stage, binding, 1, &uav);
}

void D3D11RHIContext::BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name)
{
    ID3D11SamplerState* d3d_sampler = sampler == nullptr ? nullptr : ((D3D11Sampler*)sampler)->GetD3D11SamplerState();
    SetD3DSamplers(stage, binding, 1, &d3d_sampler);
}

void OutputD3D11DebugInfo()
{
    if (!DXGIGetDebugInterface)
    {
        if (!__dxgiDebugDllLoader.Load())
        {
            LOG_ERROR("no %s.dll", __dxgiDebugDllLoader.dllname.c_str());
            return;
        }
        DXGIGetDebugInterface = (decltype(DXGIGetDebugInterface))__dxgiDebugDllLoader.FindSymbol("DXGIGetDebugInterface");
        if (!DXGIGetDebugInterface)
        {
            LOG_ERROR("cannot find DXGIGetDebugInterface in");
            return;
        }
    }

    if (!__dxgiInfoQueue)
    {
        HRESULT hr = DXGIGetDebugInterface(__uuidof(IDXGIInfoQueue), (void**)__dxgiInfoQueue.GetAddressOf());
        if (FAILED(hr))
        {
            LOG_ERROR("DXGIGetDebugInterface Error, hr=%x", hr);
            return;
        }
    }

    ComPtr<IDXGIDebug> dxgiDebug;
    if (SUCCEEDED(__dxgiInfoQueue.As(&dxgiDebug)))
    {
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
    dxgiDebug.Reset();

    UINT64 message_count = __dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    for (UINT64 i = 0; i < message_count; i++)
    {
        SIZE_T messageLength = 0;
        if (__dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength) == S_FALSE)
        {
            DXGI_INFO_QUEUE_MESSAGE* pMessage = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
            if (pMessage && __dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength) == S_OK)
            {
                LOG_INFO("DXGI Debug: %s", pMessage->pDescription);
            }
            if (pMessage)
            {
                free(pMessage);
            }
        }
    }
    __dxgiInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
}

extern "C"
{
    void MakeD3D11RHIContext(Context* context, RHIContextPtrUnique& out)
    {
        out = MakeUniquePtr<D3D11RHIContext>(context);
    }
}

SEEK_NAMESPACE_END



#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
