#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_window.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_render_view.h"
#include "rhi/d3d12/d3d12_mesh.h"
#include "rhi/d3d12/d3d12_render_state.h"
#include "rhi/d3d12/d3d12_program.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_gpu_buffer.h"
#include "rhi/d3d12/d3d12_texture.h"
#include "rhi/d3d_common/d3d_adapter.h"

#include "kernel/context.h"

#include "utils/dll_loader.h"
#include "utils/error.h"

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

D3D12Context::D3D12Context(Context* context)
    :RHIContext(context)
{
    m_pRtvDescAllocator             = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pDsvDescAllocator             = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pCbvSrvUavDescAllocator       = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pDynamicCbvSrvUavDescAllocator= MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    m_pSamplerDescAllocator         = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    m_pUploadMemoryAllocator        = MakeSharedPtr<D3D12GpuMemoryAllocator>(m_pContext, true);
    m_pReadbackMemoryAllocator      = MakeSharedPtr<D3D12GpuMemoryAllocator>(m_pContext, false);
}
D3D12Context::~D3D12Context()
{
    this->Uninit();
}
ID3D12CommandAllocator* D3D12Context::D3DRenderCmdAllocator() const
{
    return this->CurThreadContext(true).D3DCmdAllocator(m_iCurFrameIndex);
}
ID3D12GraphicsCommandList* D3D12Context::D3DRenderCmdList() const
{
    return this->CurThreadContext(true).D3DCmdList();
}

ID3D12CommandAllocator* D3D12Context::D3DLoadCmdAllocator() const
{
    return this->CurThreadContext(false).D3DCmdAllocator(m_iCurFrameIndex);
}
ID3D12GraphicsCommandList* D3D12Context::D3DLoadCmdList() const
{
    return this->CurThreadContext(false).D3DCmdList();
}
void D3D12Context::CommitLoadCmd()
{
    this->CurThreadContext(false).CommitCmd(m_pCmdQueue.Get(), m_iCurFrameIndex);
}
void D3D12Context::SyncLoadCmd()
{
    this->CurThreadContext(false).SyncCmd(m_iCurFrameIndex);
}
void D3D12Context::ResetLoadCmd()
{
    this->CurThreadContext(false).ResetCmd(m_iCurFrameIndex);
}
void D3D12Context::ForceFlush()
{
    auto& context = this->CurThreadContext(true);
    this->CommitRenderCmd(context);
    this->ResetRenderCmd(context);
    this->RestoreRenderCmdStates(context.D3DCmdList());
}
void D3D12Context::ForceFinish()
{
    //m_curvbsv curr_vbvs_.clear();
    //curr_ibv_ = { 0, 0, DXGI_FORMAT_UNKNOWN };

    //this->ForceFlush();
    //this->SyncRenderCmd();
    //this->SyncLoadCmd();

    //rtv_desc_allocator_.ClearStallPages(frame_fence_value_);
    //dsv_desc_allocator_.ClearStallPages(frame_fence_value_);
    //cbv_srv_uav_desc_allocator_.ClearStallPages(frame_fence_value_);
    //dynamic_cbv_srv_uav_desc_allocator_.ClearStallPages(frame_fence_value_);
    //sampler_desc_allocator_.ClearStallPages(frame_fence_value_);

    //upload_mem_allocator_.ClearStallPages(frame_fence_value_);
    //readback_mem_allocator_.ClearStallPages(frame_fence_value_);
    //per_frame_contexts_[curr_frame_index_].ClearStallResources();
}

void D3D12Context::IASetVertexBuffers(ID3D12GraphicsCommandList* cmd_list, uint32_t start_slot, std::span<D3D12_VERTEX_BUFFER_VIEW const> views)
{
    if ((start_slot + static_cast<size_t>(views.size()) > m_vCurrVbvs.size())
        || (memcmp(&m_vCurrVbvs[start_slot], views.data(), views.size() * sizeof(views[0])) != 0))
    {
        m_vCurrVbvs.resize(std::max(m_vCurrVbvs.size(), static_cast<size_t>(start_slot + views.size())));
        memcpy(&m_vCurrVbvs[start_slot], views.data(), views.size() * sizeof(views[0]));
        cmd_list->IASetVertexBuffers(start_slot, static_cast<uint32_t>(views.size()), views.data());
    }
}
void D3D12Context::IASetIndexBuffer(ID3D12GraphicsCommandList* cmd_list, D3D12_INDEX_BUFFER_VIEW const& view)
{
    if ((m_CurrIbv.BufferLocation != view.BufferLocation)
        || (m_CurrIbv.SizeInBytes != view.SizeInBytes)
        || (m_CurrIbv.Format != view.Format))
    {
        cmd_list->IASetIndexBuffer(&view);
        m_CurrIbv = view;
    }
}
void D3D12Context::RSSetViewports(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_VIEWPORT const> viewports)
{
    if (viewports.size() == 1)
    {
        if (memcmp(&m_CurViewport, viewports.data(), sizeof(viewports[0])) != 0)
        {
            cmd_list->RSSetViewports(1, viewports.data());
            m_CurViewport = viewports[0];
        }
    }
    else
    {
        cmd_list->RSSetViewports(static_cast<uint32_t>(viewports.size()), viewports.data());
        m_CurViewport = viewports[0];
    }
}





D3D12GpuDescriptorBlock D3D12Context::AllocRtvDescBlock(uint32_t size)
{
    return m_pRtvDescAllocator->Allocate(size);
}
void D3D12Context::DeallocRtvDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pRtvDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12Context::RenewRtvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pRtvDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12Context::AllocDsvDescBlock(uint32_t size)
{
    return m_pDsvDescAllocator->Allocate(size);
}
void D3D12Context::DeallocDsvDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pDsvDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12Context::RenewDsvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pDsvDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12Context::AllocCbvSrvUavDescBlock(uint32_t size)
{
    return m_pCbvSrvUavDescAllocator->Allocate(size);
}
void D3D12Context::DeallocCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pCbvSrvUavDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12Context::RenewCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pCbvSrvUavDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12Context::AllocDynamicCbvSrvUavDescBlock(uint32_t size)
{
    return m_pDynamicCbvSrvUavDescAllocator->Allocate(size);
}
void D3D12Context::DeallocDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pDynamicCbvSrvUavDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12Context::RenewDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pDynamicCbvSrvUavDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}

D3D12GpuDescriptorBlock D3D12Context::AllocSamplerDescBlock(uint32_t size)
{
    return m_pSamplerDescAllocator->Allocate(size);
}
void D3D12Context::DeallocSamplerDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pSamplerDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12Context::RenewSamplerDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pSamplerDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}

D3D12GpuMemoryBlock D3D12Context::AllocUploadMemBlock(uint32_t size_in_bytes, uint32_t alignment)
{
    return m_pUploadMemoryAllocator->Allocate(size_in_bytes, alignment);
}
void D3D12Context::DeallocUploadMemBlock(D3D12GpuMemoryBlock&& mem_block)
{
    m_pUploadMemoryAllocator->Deallocate(std::move(mem_block), m_iFrameFenceValue);
}
void D3D12Context::RenewUploadMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment)
{
    m_pUploadMemoryAllocator->Renew(mem_block, m_iFrameFenceValue, size_in_bytes, alignment);
}
D3D12GpuMemoryBlock D3D12Context::AllocReadbackMemBlock(uint32_t size_in_bytes, uint32_t alignment)
{
    return m_pReadbackMemoryAllocator->Allocate(size_in_bytes, alignment);
}
void D3D12Context::DeallocReadbackMemBlock(D3D12GpuMemoryBlock&& mem_block)
{
    m_pReadbackMemoryAllocator->Deallocate(std::move(mem_block), m_iFrameFenceValue);
}
void D3D12Context::RenewReadbackMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment)
{
    m_pReadbackMemoryAllocator->Renew(mem_block, m_iFrameFenceValue, size_in_bytes, alignment);
}


std::vector<D3D12_RESOURCE_BARRIER>* D3D12Context::FindResourceBarriers(ID3D12GraphicsCommandList* cmd_list, bool allow_creation)
{
    auto iter = m_vResBarriers.begin();
    for (; iter != m_vResBarriers.end(); ++iter)
    {
        if (iter->first == cmd_list)
            break;
    }

    std::vector<D3D12_RESOURCE_BARRIER>* ret;
    if (iter == m_vResBarriers.end())
    {
        if (allow_creation)
            ret = &m_vResBarriers.emplace_back(cmd_list, std::vector<D3D12_RESOURCE_BARRIER>()).second;
        else
            ret = nullptr;
    }
    else
        ret = &iter->second;
    return ret;
}
void D3D12Context::FlushResourceBarriers(ID3D12GraphicsCommandList* cmd_list)
{
    std::vector<D3D12_RESOURCE_BARRIER>* res_barriers = this->FindResourceBarriers(cmd_list, false);
    if (res_barriers && !res_barriers->empty())
    {
        cmd_list->ResourceBarrier(static_cast<UINT>(res_barriers->size()), res_barriers->data());
        res_barriers->clear();
    }
}
void D3D12Context::AddResourceBarrier(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_RESOURCE_BARRIER> barriers)
{
    std::vector<D3D12_RESOURCE_BARRIER>* res_barriers = this->FindResourceBarriers(cmd_list, true);
    res_barriers->insert(res_barriers->end(), barriers.begin(), barriers.end());
}
void D3D12Context::AddStallResource(ID3D12ResourcePtr const& resource)
{
    if (resource)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_vStallResources.push_back(resource);
    }
}
D3D12Context::PerThreadContext& D3D12Context::CurThreadContext(bool is_render_context) const
{
    std::thread::id thread_id = std::this_thread::get_id();
    auto& thread_cmd_contexts = is_render_context ? m_vRenderThreadCmdContexts : m_vLoadThreadCmdContexts;
    auto iter = std::find_if(thread_cmd_contexts.begin(), thread_cmd_contexts.end(),
        [&thread_id](std::unique_ptr<PerThreadContext> const& context) { return context->ThreadID() == thread_id; });
    if (iter == thread_cmd_contexts.end())
    {
        auto new_context = MakeUniquePtr<PerThreadContext>(m_pDevice.Get(), m_pFrameFence);
        if (!is_render_context)
        {
            new_context->D3DCmdList()->Close();
        }

        thread_cmd_contexts.emplace_back(std::move(new_context));
        iter = thread_cmd_contexts.end() - 1;
    }
    return *(*iter);
}
void D3D12Context::CommitRenderCmd(PerThreadContext& context)
{
    this->CommitRenderCmd(this->CurThreadContext(true));
}
void D3D12Context::SyncRenderCmd(PerThreadContext& context)
{
    this->SyncRenderCmd(this->CurThreadContext(true));
}
void D3D12Context::ResetRenderCmd(PerThreadContext& context)
{
    context.SyncCmd(m_iCurFrameIndex);
}








SResult D3D12Context::Init()
{
    SEEK_RETIF_FAIL(DxgiHelper::Init(m_pContext->GetPreferredAdapter(), m_pContext->EnableDebug()));
    do { 
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

        D3D_FEATURE_LEVEL feature_levels[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0
        };

        uint32_t feature_level_start_index = 0;
        if (m_iDxgiSubVer < 4)
        {
            feature_level_start_index = 2;
        }
        HRESULT hr = S_OK;
        uint32_t feature_level_count = sizeof(feature_levels) / sizeof(D3D_FEATURE_LEVEL);
        for (; feature_level_start_index < feature_level_count; feature_level_start_index++)
        {
            hr = FUNC_D3D12CreateDevice(m_vAdapterList[m_iCurAdapterNo]->DXGIAdapter(),
                feature_levels[feature_level_start_index], __uuidof(ID3D12Device),
                (void**)m_pDevice.GetAddressOf());
            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
        {
            LOG_ERROR("D3D12CreateDevice Error, hr:%x", hr);
            break;
        }
        LOG_INFO("device supported feature level %s", GetD3D12FeatureLevelStr(feature_levels[feature_level_start_index]));

        // Init Grphics Resources
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        m_pDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(m_pCmdQueue.ReleaseAndGetAddressOf()));
        
        m_pFrameFence = this->CreateFence();


        {
            D3D12_INDIRECT_ARGUMENT_DESC desc;
            desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
            D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
            cmd_signature_desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
            cmd_signature_desc.NumArgumentDescs = 1;
            cmd_signature_desc.pArgumentDescs = &desc;
            cmd_signature_desc.NodeMask = 1;
            hr = m_pDevice->CreateCommandSignature(&cmd_signature_desc, nullptr, IID_PPV_ARGS(m_pDrawIndirectSign.ReleaseAndGetAddressOf()));
            SEEK_RETIF_FAIL(hr);
        }
        {
            D3D12_INDIRECT_ARGUMENT_DESC desc;
            desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
            D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
            cmd_signature_desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
            cmd_signature_desc.NumArgumentDescs = 1;
            cmd_signature_desc.pArgumentDescs = &desc;
            cmd_signature_desc.NodeMask = 1;
            hr = m_pDevice->CreateCommandSignature(&cmd_signature_desc, nullptr, IID_PPV_ARGS(m_pDrawIndexedIndirectSign.ReleaseAndGetAddressOf()));
            SEEK_RETIF_FAIL(hr);
        }
        {
            D3D12_INDIRECT_ARGUMENT_DESC desc;
            desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
            D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
            cmd_signature_desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
            cmd_signature_desc.NumArgumentDescs = 1;
            cmd_signature_desc.pArgumentDescs = &desc;
            cmd_signature_desc.NodeMask = 1;
            hr = m_pDevice->CreateCommandSignature(&cmd_signature_desc, nullptr, IID_PPV_ARGS(m_pDispatchIndirectSign.ReleaseAndGetAddressOf()));
            SEEK_RETIF_FAIL(hr);
        }

        SEEK_RETIF_FAIL(this->CheckCapabilitySetSupport());

        // Initialize null SRV/UAV descriptors for empty table slots
        {
            m_NullSrvUavDescBlock = m_pCbvSrvUavDescAllocator->Allocate(2);
            m_hNullSrvHandle = m_NullSrvUavDescBlock.CpuHandle();
            m_hNullUavHandle = m_NullSrvUavDescBlock.CpuHandle();
            m_hNullUavHandle.ptr += m_pCbvSrvUavDescAllocator->DescriptorSize();

            D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
            nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            nullSrvDesc.Texture2D.MipLevels = 1;
            m_pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, m_hNullSrvHandle);

            D3D12_UNORDERED_ACCESS_VIEW_DESC nullUavDesc = {};
            nullUavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            nullUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            m_pDevice->CreateUnorderedAccessView(nullptr, nullptr, &nullUavDesc, m_hNullUavHandle);
        }

        SEEK_RETIF_FAIL(this->CreateRootSignature());

        return S_Success;
    } while (0);

    return S_Success;
}
void D3D12Context::Uninit()
{

}
SResult D3D12Context::CheckCapabilitySetSupport()
{
    return S_Success;
}
SResult D3D12Context::AttachNativeWindow(std::string const& name, void* native_wnd)
{
    SResult res = S_Success;
    D3DAdapterPtr pAdapter = this->ActiveAdapter();
    if (native_wnd)
    {
        D3D12WindowPtr win = MakeSharedPtr<D3D12Window>(m_pContext);
        res = win->Create(pAdapter.get(), name, native_wnd);
        if (SEEK_CHECKFAILED(res))
            return res;
        this->BindRHIFrameBuffer(win);
        m_pScreenRHIFrameBuffer = m_pCurRHIFrameBuffer;
    }
    return res;
}







SResult D3D12Context::BeginFrame()
{
    return S_Success;
}
SResult D3D12Context::EndFrame()
{
    return S_Success;
}

/******************************************************************************
* Resource Binding
*******************************************************************************/
void D3D12Context::BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIGpuBuffer* cbuffer, const char* name)
{
    if (!cbuffer || binding >= 5) return;

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();
    const D3D12GpuBuffer* d3dBuf = static_cast<const D3D12GpuBuffer*>(cbuffer);
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddr = d3dBuf->GpuVirtualAddress();

    switch (binding)
    {
    case 0: cmd_list->SetGraphicsRootConstantBufferView(ROOT_PARAM_CBV_B0, gpuAddr); break;
    case 1: cmd_list->SetGraphicsRootConstantBufferView(ROOT_PARAM_CBV_B1, gpuAddr); break;
    case 2: cmd_list->SetGraphicsRootConstantBufferView(ROOT_PARAM_CBV_B2, gpuAddr); break;
    case 3: cmd_list->SetGraphicsRootConstantBufferView(ROOT_PARAM_CBV_B3, gpuAddr); break;
    case 4: cmd_list->SetGraphicsRootConstantBufferView(ROOT_PARAM_CBV_B4, gpuAddr); break;
    default: break;
    }
}

void D3D12Context::BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name)
{
    if (!texture || binding >= MAX_DESCRIPTOR_SRV) return;
    D3D12Texture* d3dTex = (D3D12Texture*)(texture);
    D3D12SrvPtr const& srv = d3dTex->GetD3DSrv();
    if (srv && srv->Handle().ptr)
    {
        m_srvCache.pendingHandles[binding] = srv->Handle();
        if (binding >= m_srvCache.maxBinding)
            m_srvCache.maxBinding = binding + 1;
        m_srvCache.dirty = true;
    }
}

void D3D12Context::BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name)
{
    // Static samplers s0/s1/s2 are defined in the root signature and always active.
    // Dynamic sampler binding via descriptor table is not yet implemented.
}

void D3D12Context::BindRHISrv(ShaderType stage, uint32_t binding, const RHIShaderResourceView* srv, const char* name)
{
    if (!srv || binding >= MAX_DESCRIPTOR_SRV) return;

    D3D12ShaderResourceView* d3dSrv = (D3D12ShaderResourceView*)(srv);
    D3D12Srv* handle = d3dSrv->GetD3DSrv();
    if (!handle) return;

    m_srvCache.pendingHandles[binding] = handle->Handle();
    if (binding >= m_srvCache.maxBinding)
        m_srvCache.maxBinding = binding + 1;
    m_srvCache.dirty = true;
}

void D3D12Context::BindRHIUav(ShaderType stage, uint32_t binding, const RHIUnorderedAccessView* uav, const char* name)
{
    if (!uav || binding >= MAX_DESCRIPTOR_UAV) return;

    D3D12UnorderedAccessView* d3dUav = (D3D12UnorderedAccessView*)(uav);
    D3D12Uav* handle = d3dUav->GetD3DUav();
    if (!handle) return;

    m_uavCache.pendingHandles[binding] = handle->Handle();
    if (binding >= m_uavCache.maxBinding)
        m_uavCache.maxBinding = binding + 1;
    m_uavCache.dirty = true;
}

void D3D12Context::BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name)
{
    if (!rw_texture || binding >= MAX_DESCRIPTOR_UAV) return;
    D3D12Texture* d3dTex = (D3D12Texture*)(rw_texture);
    D3D12UavPtr const& uav = d3dTex->GetD3DUav();
    if (uav && uav->Handle().ptr)
    {
        m_uavCache.pendingHandles[binding] = uav->Handle();
        if (binding >= m_uavCache.maxBinding)
            m_uavCache.maxBinding = binding + 1;
        m_uavCache.dirty = true;
    }
}

void D3D12Context::FlushDescriptorTables(ID3D12GraphicsCommandList* cmd_list)
{
    ID3D12Device* device = m_pDevice.Get();
    uint32_t descriptorSize = m_pDynamicCbvSrvUavDescAllocator->DescriptorSize();

    uint32_t srvCount = Math::Max(m_srvCache.maxBinding, 1u);
    uint32_t uavCount = Math::Max(m_uavCache.maxBinding, 1u);

    // Allocate SRV block if needed
    if (m_srvCache.dirty || !m_srvCache.tableSet)
    {
        if (m_srvCache.block.GetSize() < srvCount)
        {
            if (m_srvCache.block.GetHeap())
                m_pDynamicCbvSrvUavDescAllocator->Deallocate(std::move(m_srvCache.block), m_iFrameFenceValue);
            m_srvCache.block = m_pDynamicCbvSrvUavDescAllocator->Allocate(srvCount);
            m_srvCache.dirty = true;
        }
    }

    // Allocate UAV block if needed
    if (m_uavCache.dirty || !m_uavCache.tableSet)
    {
        if (m_uavCache.block.GetSize() < uavCount)
        {
            if (m_uavCache.block.GetHeap())
                m_pDynamicCbvSrvUavDescAllocator->Deallocate(std::move(m_uavCache.block), m_iFrameFenceValue);
            m_uavCache.block = m_pDynamicCbvSrvUavDescAllocator->Allocate(uavCount);
            m_uavCache.dirty = true;
        }
    }

    // Set descriptor heap BEFORE setting any descriptor tables
    ID3D12DescriptorHeap* pHeap = m_srvCache.block.GetHeap();
    if (!pHeap) pHeap = m_uavCache.block.GetHeap();
    if (pHeap && m_pCurSrvUavHeap.Get() != pHeap)
    {
        cmd_list->SetDescriptorHeaps(1, &pHeap);
        m_pCurSrvUavHeap = pHeap;
    }

    // Flush SRV descriptor table
    if (m_srvCache.dirty && m_srvCache.maxBinding > 0)
    {
        for (uint32_t i = 0; i < srvCount; i++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE src = (i < m_srvCache.maxBinding && m_srvCache.pendingHandles[i].ptr) ? m_srvCache.pendingHandles[i] : m_hNullSrvHandle;
            D3D12_CPU_DESCRIPTOR_HANDLE dst = m_srvCache.block.CpuHandle();
            dst.ptr += i * descriptorSize;
            device->CopyDescriptorsSimple(1, dst, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        m_srvCache.dirty = false;
    }

    if (m_srvCache.block.GetHeap())
    {
        cmd_list->SetGraphicsRootDescriptorTable(ROOT_PARAM_SRV_TABLE, m_srvCache.block.GpuHandle());
        m_srvCache.tableSet = true;
    }

    // Flush UAV descriptor table
    if (m_uavCache.dirty && m_uavCache.maxBinding > 0)
    {
        for (uint32_t i = 0; i < uavCount; i++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE src = (i < m_uavCache.maxBinding && m_uavCache.pendingHandles[i].ptr) ? m_uavCache.pendingHandles[i] : m_hNullUavHandle;
            D3D12_CPU_DESCRIPTOR_HANDLE dst = m_uavCache.block.CpuHandle();
            dst.ptr += i * descriptorSize;
            device->CopyDescriptorsSimple(1, dst, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        m_uavCache.dirty = false;
    }

    if (m_uavCache.block.GetHeap())
    {
        cmd_list->SetGraphicsRootDescriptorTable(ROOT_PARAM_UAV_TABLE, m_uavCache.block.GpuHandle());
        m_uavCache.tableSet = true;
    }
}

SResult D3D12Context::BeginRenderPass(const RenderPassInfo& renderPassInfo)
{
    if (!renderPassInfo.fb)
        return ERR_INVALID_DATA;

    // Lazy frame init: if cmd list isn't open, reset and begin
    static bool s_firstFrame = true;
    auto& threadCtx = this->CurThreadContext(true);
    ID3D12GraphicsCommandList* cmd_list = threadCtx.D3DCmdList();

    // Reset state tracking each frame
    if (s_firstFrame)
    {
        s_firstFrame = false;
        // cmd list is already open from AttachNativeWindow/SwapBuffers
    }

    SResult ret = renderPassInfo.fb->Bind();
    if (SEEK_CHECKFAILED(ret))
    {
        LOG_ERROR("bind RHIFrameBuffer fail, ret:%x", ret);
        return ret;
    }
    m_pCurrentRHIFrameBuffer = static_cast<D3D12FrameBuffer*>(renderPassInfo.fb);

    // Apply barriers and set render targets
    m_pCurrentRHIFrameBuffer->BindBarrier(cmd_list);
    this->FlushResourceBarriers(cmd_list);
    m_pCurrentRHIFrameBuffer->SetRenderTargets(cmd_list);

    // Reset PSO tracking and descriptor caches for new pass
    m_pCurPso = nullptr;
    m_pCurrGraphicsRootSignature = nullptr;
    m_srvCache.Reset();
    m_uavCache.Reset();

    D3D12_VIEWPORT d3dViewport;
    d3dViewport.TopLeftX = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().left;
    d3dViewport.TopLeftY = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().top;
    d3dViewport.Width = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().width;
    d3dViewport.Height = (FLOAT)m_pCurrentRHIFrameBuffer->GetViewport().height;
    d3dViewport.MinDepth = 0.0;
    d3dViewport.MaxDepth = 1.0;
    cmd_list->RSSetViewports(1, &d3dViewport);

    // Clear if requested
    m_pCurrentRHIFrameBuffer->Clear();

    return S_Success;
}
SResult D3D12Context::Render(RHIProgram* program, RHIMeshPtr const& mesh)
{
    if (!m_pCurrentRHIFrameBuffer)
    {
        LOG_ERROR("no RHIFrameBuffer is bound, call Render between BeginRenderPass/EndRenderPass");
        return ERR_INVALID_INVOKE_FLOW;
    }

    SResult ret = S_Success;

    RHIRenderState* rs = mesh->GetRenderState().get();

    SEEK_RETIF_FAIL(((D3D12RenderState*)(rs))->Active());
    SEEK_RETIF_FAIL(((D3D12Program*)(program))->Active());

    D3D12Mesh& d3d_mesh = static_cast<D3D12Mesh&>(*mesh);
    SEEK_RETIF_FAIL(d3d_mesh.Active(program));

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();

    {
        uint32_t instance_count = mesh->IsInstanceRendering() ? 1 : mesh->GetInstanceCount();
        this->UpdateRenderPso(cmd_list, program, mesh);
        if (mesh->IsUseIndices())
        {            
            uint32_t index_count = mesh->GetNumIndices();
            cmd_list->DrawIndexedInstanced(index_count, instance_count, 0, 0, 0);
        }
        else
        {            
            uint32_t vertex_count = mesh->GetNumVertex();
            cmd_list->DrawInstanced(vertex_count, instance_count, 0, 0);
        }
    }

    return S_Success;
}
SResult D3D12Context::EndRenderPass()
{
    return S_Success;
}

/******************************************************************************
* Compute Pass
*******************************************************************************/
void D3D12Context::BeginComputePass(const ComputePassInfo& computePassInfo)
{
    m_srvCache.Reset();
    m_uavCache.Reset();
}

SResult D3D12Context::Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z)
{
    if (!program) return ERR_INVALID_ARG;

    D3D12Program* d3dProgram = static_cast<D3D12Program*>(program);
    d3dProgram->Active();

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();
    this->UpdateComputePso(cmd_list, program, nullptr);
    cmd_list->Dispatch(x, y, z);

    return S_Success;
}

void D3D12Context::EndComputePass()
{
}

SResult D3D12Context::DispatchIndirect(RHIProgram* program, RHIGpuBufferPtr indirectBuf)
{
    if (!program || !indirectBuf) return ERR_INVALID_ARG;

    D3D12Program* d3dProgram = static_cast<D3D12Program*>(program);
    d3dProgram->Active();

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();
    this->UpdateComputePso(cmd_list, program, nullptr);

    const D3D12GpuBuffer* d3dBuf = static_cast<const D3D12GpuBuffer*>(indirectBuf.get());
    ID3D12Resource* d3dRes = d3dBuf->GetD3DResource();
    cmd_list->ExecuteIndirect(m_pDispatchIndirectSign.Get(), 1,
        d3dRes, 0, nullptr, 0);

    return S_Success;
}

SResult D3D12Context::DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIGpuBufferPtr indirectBuf, MeshTopologyType type)
{
    if (!program || !indirectBuf) return ERR_INVALID_ARG;

    D3D12Program* d3dProgram = static_cast<D3D12Program*>(program);
    d3dProgram->Active();

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();
    this->UpdateRenderPso(cmd_list, program, nullptr);

    const D3D12GpuBuffer* d3dBuf = static_cast<const D3D12GpuBuffer*>(indirectBuf.get());
    ID3D12Resource* d3dRes = d3dBuf->GetD3DResource();
    cmd_list->ExecuteIndirect(m_pDrawIndirectSign.Get(), 1,
        d3dRes, 0, nullptr, 0);

    return S_Success;
}

SResult D3D12Context::DrawInstanced(RHIProgram* program, RHIRenderStatePtr rs, MeshTopologyType type, uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
    if (!program) return ERR_INVALID_ARG;

    D3D12Program* d3dProgram = static_cast<D3D12Program*>(program);
    d3dProgram->Active();

    ID3D12GraphicsCommandList* cmd_list = this->D3DRenderCmdList();
    this->UpdateRenderPso(cmd_list, program, nullptr);
    cmd_list->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);

    return S_Success;
}

/******************************************************************************
* Root Signature
*******************************************************************************/
SResult D3D12Context::CreateRootSignature()
{
    // Root parameters:
    // [0-4] CBV root descriptors (b0-b4)
    // [5]   SRV descriptor table (t0+, unbounded)
    std::array<D3D12_ROOT_PARAMETER, NUM_ROOT_PARAMS> rootParams = {};

    for (uint32_t i = 0; i < 5; i++)
    {
        rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParams[i].Descriptor.ShaderRegister = i;
        rootParams[i].Descriptor.RegisterSpace = 0;
        rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    // SRV descriptor table: one range for t0+, unbounded
    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = (UINT)-1; // unbounded
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[ROOT_PARAM_SRV_TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[ROOT_PARAM_SRV_TABLE].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[ROOT_PARAM_SRV_TABLE].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[ROOT_PARAM_SRV_TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // UAV descriptor table: one range for u0+, unbounded
    D3D12_DESCRIPTOR_RANGE uavRange = {};
    uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uavRange.NumDescriptors = (UINT)-1; // unbounded
    uavRange.BaseShaderRegister = 0;
    uavRange.RegisterSpace = 0;
    uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[ROOT_PARAM_UAV_TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[ROOT_PARAM_UAV_TABLE].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[ROOT_PARAM_UAV_TABLE].DescriptorTable.pDescriptorRanges = &uavRange;
    rootParams[ROOT_PARAM_UAV_TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Static samplers
    std::array<D3D12_STATIC_SAMPLER_DESC, NUM_STATIC_SAMPLERS> staticSamplers = {};

    // s0: linear wrap sampler
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].MipLODBias = 0;
    staticSamplers[0].MaxAnisotropy = 1;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[0].MinLOD = 0;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].RegisterSpace = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // s1: point clamp sampler
    staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].MipLODBias = 0;
    staticSamplers[1].MaxAnisotropy = 1;
    staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    staticSamplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[1].MinLOD = 0;
    staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[1].ShaderRegister = 1;
    staticSamplers[1].RegisterSpace = 0;
    staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // s2: shadow comparison sampler
    staticSamplers[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    staticSamplers[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[2].MipLODBias = 0;
    staticSamplers[2].MaxAnisotropy = 1;
    staticSamplers[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    staticSamplers[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[2].MinLOD = 0;
    staticSamplers[2].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[2].ShaderRegister = 2;
    staticSamplers[2].RegisterSpace = 0;
    staticSamplers[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // Build the root signature
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = NUM_ROOT_PARAMS;
    rootSigDesc.pParameters = rootParams.data();
    rootSigDesc.NumStaticSamplers = NUM_STATIC_SAMPLERS;
    rootSigDesc.pStaticSamplers = staticSamplers.data();
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlobPtr pSignature;
    ID3DBlobPtr pError;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf());
    if (FAILED(hr))
    {
        if (pError)
            LOG_ERROR("D3D12SerializeRootSignature failed: %s", (char*)pError->GetBufferPointer());
        return ERR_INVALID_DATA;
    }

    hr = m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(),
        pSignature->GetBufferSize(), IID_PPV_ARGS(m_pGraphicsRootSignature.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        LOG_ERROR("CreateRootSignature failed, hr=%x", hr);
        return ERR_INVALID_DATA;
    }

    // Compute root signature (same layout, no IA flag)
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    pSignature.Reset();
    pError.Reset();
    hr = D3D12SerializeRootSignature(&rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf());
    if (FAILED(hr)) { return ERR_INVALID_DATA; }

    hr = m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(),
        pSignature->GetBufferSize(), IID_PPV_ARGS(m_pComputeRootSignature.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        LOG_ERROR("CreateRootSignature(compute) failed, hr=%x", hr);
        return ERR_INVALID_DATA;
    }

    return S_Success;
}

void D3D12Context::UpdateRenderPso(ID3D12GraphicsCommandList* cmd_list, RHIProgram* program, RHIMeshPtr const& mesh)
{
    D3D12RenderState* rs = static_cast<D3D12RenderState*>(mesh->GetRenderState().get());
    D3D12FrameBuffer* fb = this->m_pCurrentRHIFrameBuffer;
    D3D12Mesh& d3dMesh = static_cast<D3D12Mesh&>(*mesh);

    if (!fb) return;

    ID3D12PipelineState* pso = rs->GetGraphicPso(d3dMesh, *program, *fb);
    if (!pso) return;

    if (m_pCurPso != pso)
    {
        cmd_list->SetPipelineState(pso);
        m_pCurPso = pso;
    }

    if (m_pCurrGraphicsRootSignature != m_pGraphicsRootSignature.Get())
    {
        cmd_list->SetGraphicsRootSignature(m_pGraphicsRootSignature.Get());
        m_pCurrGraphicsRootSignature = m_pGraphicsRootSignature.Get();
    }

    // Ensure descriptor tables are committed before draw
    this->FlushDescriptorTables(cmd_list);
}

void D3D12Context::UpdateComputePso(ID3D12GraphicsCommandList* cmd_list, RHIProgram* program, RHIMeshPtr const& mesh)
{
    if (m_CurComputeRootSignature != m_pComputeRootSignature.Get())
    {
        cmd_list->SetComputeRootSignature(m_pComputeRootSignature.Get());
        m_CurComputeRootSignature = m_pComputeRootSignature.Get();
    }
    this->FlushDescriptorTables(cmd_list);
}

/******************************************************************************
* D3D12Context::PerThreadContext
*******************************************************************************/
D3D12Context::PerThreadContext::PerThreadContext(ID3D12Device* d3d_device, RHIFencePtr const& frame_fence)
    : m_ThreadId(std::this_thread::get_id()), m_pFrameFence(frame_fence)
{
    for (auto& context : m_vPerFrameContexts)
    {
        SEEK_THROW_IFFAIL(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
            IID_PPV_ARGS(context.d3d_cmd_allocator.ReleaseAndGetAddressOf())));
    }
    SEEK_THROW_IFFAIL(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_vPerFrameContexts[0].d3d_cmd_allocator.Get(),
        nullptr, IID_PPV_ARGS(m_pD3dCmdList.ReleaseAndGetAddressOf())));
}
D3D12Context::PerThreadContext::~PerThreadContext()
{
    if (auto fence = m_pFrameFence.lock())
    {
        uint64_t max_fence_val = 0;
        for (auto const& context : m_vPerFrameContexts)
        {
            max_fence_val = std::max(max_fence_val, context.fence_value);
        }
        fence->Wait(max_fence_val);
        m_pFrameFence.reset();
    }

    m_pD3dCmdList.Reset();
    for (auto& context : m_vPerFrameContexts)
    {
        context.d3d_cmd_allocator.Reset();
        context.fence_value = 0;
    }
}
void D3D12Context::PerThreadContext::CommitCmd(ID3D12CommandQueue* d3d_cmd_queue, uint32_t frame_index)
{
    SEEK_THROW_IFFAIL(m_pD3dCmdList->Close());
    ID3D12CommandList* cmd_lists[] = { m_pD3dCmdList.Get() };
    d3d_cmd_queue->ExecuteCommandLists(static_cast<uint32_t>(std::size(cmd_lists)), cmd_lists);
    m_vPerFrameContexts[frame_index].fence_value = static_cast<D3D12Fence&>(*m_pFrameFence.lock()).Signal(d3d_cmd_queue);
}
void D3D12Context::PerThreadContext::SyncCmd(uint32_t frame_index)
{
    m_pFrameFence.lock()->Wait(m_vPerFrameContexts[frame_index].fence_value);
}

void D3D12Context::PerThreadContext::ResetCmd(uint32_t frame_index)
{
    m_pD3dCmdList->Reset(this->D3DCmdAllocator(frame_index), nullptr);
}
void D3D12Context::PerThreadContext::Reset(uint32_t frame_index)
{
    this->SyncCmd(frame_index);
    this->D3DCmdAllocator(frame_index)->Reset();
}
ID3D12CommandAllocator* D3D12Context::PerThreadContext::D3DCmdAllocator(uint32_t frame_index) const
{
    return m_vPerFrameContexts[frame_index].d3d_cmd_allocator.Get();
}
ID3D12GraphicsCommandList* D3D12Context::PerThreadContext::D3DCmdList() const
{
    return m_pD3dCmdList.Get();
}
uint64_t D3D12Context::PerThreadContext::FrameFenceValue(uint32_t frame_index) const
{
    return m_vPerFrameContexts[frame_index].fence_value;
}

D3D12Context::PerFrameContext::~PerFrameContext()
{
    m_vStallResources.clear();
}
void D3D12Context::PerFrameContext::AddStallResource(ID3D12ResourcePtr const& resource)
{
    if (resource)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_vStallResources.push_back(resource);
    }
}
void D3D12Context::PerFrameContext::ClearStallResources()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_vStallResources.clear();
}







extern "C"
{
    void MakeD3D12Context(Context* context, RHIContextPtrUnique& out)
    {
        out = MakeUniquePtr<D3D12Context>(context);
    }
}
SEEK_NAMESPACE_END