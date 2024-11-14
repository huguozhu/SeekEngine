#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_window.h"
#include "rhi/d3d11_rhi/d3d11_program.h"
#include "rhi/d3d11_rhi/d3d11_mesh.h"
#include "rhi/d3d11_rhi/d3d11_render_buffer.h"
#include "rhi/d3d11_rhi/d3d11_render_state.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "rhi/d3d11_rhi/d3d11_translate.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_fence.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"

#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_program.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 6     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static DllLoader s_d3d11("d3d11.dll");
static decltype(&::D3D11CreateDevice) FUNC_D3D11CreateDevice = nullptr;

const char* GetD3D11FeatureLevelStr(D3D_FEATURE_LEVEL feature_level)
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
    : RHIContext(context), m_pDevice(nullptr), m_pDeviceContext(nullptr)
{    
}

D3D11RHIContext::~D3D11RHIContext()
{
    Uninit();
}

SResult D3D11RHIContext::Init()
{
    SEEK_RETIF_FAIL(DxgiHelper::Init(m_pContext->GetPreferredAdapter(), m_pContext->IsDebug()));
    do {        

        if (!s_d3d11.Load())
        {
            LOG_ERROR("load %s fail", s_d3d11.dllname.c_str());
            return ERR_NOT_SUPPORT;
        }

        if (!FUNC_D3D11CreateDevice)
        {
            FUNC_D3D11CreateDevice = (decltype(FUNC_D3D11CreateDevice))s_d3d11.FindSymbol("D3D11CreateDevice");
            if (!FUNC_D3D11CreateDevice)
            {
                LOG_ERROR("no D3D11CreateDevice entry point");
                return ERR_NOT_SUPPORT;
            }
        }

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

        HRESULT hr = S_OK;
        D3D_FEATURE_LEVEL out_feature_level;
        uint32_t feature_level_count = sizeof(feature_levels) / sizeof(D3D_FEATURE_LEVEL);
        while (feature_level_start_index < feature_level_count)
        {
            hr = FUNC_D3D11CreateDevice(m_vAdapterList[m_iCurAdapterNo]->DXGIAdapter(),
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
        LOG_INFO("device supported feature level %s", GetD3D11FeatureLevelStr(out_feature_level));

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
    DxgiHelper::Uninit();
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
    D3DAdapterPtr pAdapter = this->ActiveAdapter();
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
SResult D3D11RHIContext::CopyTextureRegion(RHITexturePtr tex_src, RHITexturePtr tex_dst, int32_t dst_x, int32_t dst_y, int32_t dst_z)
{
    D3D11Texture* src = dynamic_cast<D3D11Texture*>(tex_src.get());
    D3D11Texture* dst = dynamic_cast<D3D11Texture*>(tex_dst.get());
    uint32_t src_width = tex_src->Width();
    uint32_t src_height = tex_src->Height();
    uint32_t src_depth = tex_src->Depth();
    uint32_t dst_width = tex_dst->Width();
    uint32_t dst_height = tex_dst->Height();
    uint32_t dst_depth = tex_dst->Depth();

    if (dst_x + src_width <= 0 ||
        dst_y + src_height <= 0 ||
        dst_z + src_depth <= 0)
        return ERR_INVALID_ARG;
    if (dst_x >= dst_width  || 
        dst_y >= dst_height ||
        dst_z >= dst_depth  )
        return ERR_INVALID_ARG;

    INT left = 0;
    INT top = 0;
    INT front = 0;
    INT right = Math::Min(src_width, dst_width - dst_x);
    INT bottom = Math::Min(src_height, dst_height - dst_y);
    INT back = Math::Min(src_depth, dst_depth - dst_z);

    if (dst_x < 0)
    {
        left = src_width + dst_x;
        right = src_width;
        dst_x = 0;
    }
    if (dst_y < 0)
    {
        top = src_height + dst_y;
        bottom = src_height;
        dst_y;
    }
    if (dst_z < 0)
    {
        front = src_depth + dst_z;
        back = src_depth;
        dst_z = 0;
    }

    D3D11_BOX box = {
        (UINT)left,  (UINT)top,     (UINT)front,
        (UINT)right, (UINT)bottom,  (UINT)back};

    m_pDeviceContext->CopySubresourceRegion(dst->GetD3DTexture(), 0, dst_x, dst_y, dst_z, src->GetD3DTexture(), 0, &box);

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
RHIFencePtr D3D11RHIContext::CreateFence()
{
    return MakeSharedPtr<D3D11RHIFence>(m_pContext);
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
