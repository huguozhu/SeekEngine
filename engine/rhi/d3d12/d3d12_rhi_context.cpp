#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d_common/d3d_adapter.h"
#include "rhi/d3d12/d3d12_window.h"
#include "rhi/base/rhi_fence.h"
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
    m_pRtvDescAllocator             = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pDsvDescAllocator             = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pCbvSrvUavDescAllocator       = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_pDynamicCbvSrvUavDescAllocator= MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    m_pSamplerDescAllocator         = MakeSharedPtr<D3D12GpuDescriptorAllocator>(m_pContext, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    m_pUploadMemoryAllocator        = MakeSharedPtr<D3D12GpuMemoryAllocator>(m_pContext, true);
    m_pReadbackMemoryAllocator      = MakeSharedPtr<D3D12GpuMemoryAllocator>(m_pContext, false);
}
D3D12RHIContext::~D3D12RHIContext()
{
    this->Uninit();
}
D3D12GpuDescriptorBlock D3D12RHIContext::AllocRtvDescBlock(uint32_t size)
{
    return m_pRtvDescAllocator->Allocate(size);
}
void D3D12RHIContext::DeallocRtvDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pRtvDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewRtvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pRtvDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12RHIContext::AllocDsvDescBlock(uint32_t size)
{
    return m_pDsvDescAllocator->Allocate(size);
}
void D3D12RHIContext::DeallocDsvDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pDsvDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewDsvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pDsvDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12RHIContext::AllocCbvSrvUavDescBlock(uint32_t size)
{
    return m_pCbvSrvUavDescAllocator->Allocate(size);
}
void D3D12RHIContext::DeallocCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pCbvSrvUavDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pCbvSrvUavDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}
D3D12GpuDescriptorBlock D3D12RHIContext::AllocDynamicCbvSrvUavDescBlock(uint32_t size)
{
    return m_pDynamicCbvSrvUavDescAllocator->Allocate(size);
}
void D3D12RHIContext::DeallocDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pDynamicCbvSrvUavDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pDynamicCbvSrvUavDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}

D3D12GpuDescriptorBlock D3D12RHIContext::AllocSamplerDescBlock(uint32_t size)
{
    return m_pSamplerDescAllocator->Allocate(size);
}
void D3D12RHIContext::DeallocSamplerDescBlock(D3D12GpuDescriptorBlock&& desc_block)
{
    m_pSamplerDescAllocator->Deallocate(std::move(desc_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewSamplerDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
    m_pSamplerDescAllocator->Renew(desc_block, m_iFrameFenceValue, size);
}

D3D12GpuMemoryBlock D3D12RHIContext::AllocUploadMemBlock(uint32_t size_in_bytes, uint32_t alignment)
{
    return m_pUploadMemoryAllocator->Allocate(size_in_bytes, alignment);
}
void D3D12RHIContext::DeallocUploadMemBlock(D3D12GpuMemoryBlock&& mem_block)
{
    m_pUploadMemoryAllocator->Deallocate(std::move(mem_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewUploadMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment)
{
    m_pUploadMemoryAllocator->Renew(mem_block, m_iFrameFenceValue, size_in_bytes, alignment);
}
D3D12GpuMemoryBlock D3D12RHIContext::AllocReadbackMemBlock(uint32_t size_in_bytes, uint32_t alignment)
{
    return m_pReadbackMemoryAllocator->Allocate(size_in_bytes, alignment);
}
void D3D12RHIContext::DeallocReadbackMemBlock(D3D12GpuMemoryBlock&& mem_block)
{
    m_pReadbackMemoryAllocator->Deallocate(std::move(mem_block), m_iFrameFenceValue);
}
void D3D12RHIContext::RenewReadbackMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment)
{
    m_pReadbackMemoryAllocator->Renew(mem_block, m_iFrameFenceValue, size_in_bytes, alignment);
}


std::vector<D3D12_RESOURCE_BARRIER>* D3D12RHIContext::FindResourceBarriers(ID3D12GraphicsCommandList* cmd_list, bool allow_creation)
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
void D3D12RHIContext::FlushResourceBarriers(ID3D12GraphicsCommandList* cmd_list)
{
    std::vector<D3D12_RESOURCE_BARRIER>* res_barriers = this->FindResourceBarriers(cmd_list, false);
    if (res_barriers && !res_barriers->empty())
    {
        cmd_list->ResourceBarrier(static_cast<UINT>(res_barriers->size()), res_barriers->data());
        res_barriers->clear();
    }
}
void D3D12RHIContext::AddResourceBarrier(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_RESOURCE_BARRIER> barriers)
{
    std::vector<D3D12_RESOURCE_BARRIER>* res_barriers = this->FindResourceBarriers(cmd_list, false);
    res_barriers->insert(res_barriers->end(), barriers.begin(), barriers.end());
}
void D3D12RHIContext::AddStallResource(ID3D12ResourcePtr const& resource)
{
    if (resource)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_vStallResources.push_back(resource);
    }
}



SResult D3D12RHIContext::Init()
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

        {
            // Init Grphics Resources
            D3D12_COMMAND_QUEUE_DESC queue_desc = {};
            queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            m_pDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(m_pCommandQueueGraphics.ReleaseAndGetAddressOf()));

            for (uint32_t i = 0; i < m_iBackBufferCount; i++)
            {
                SEEK_THROW_IFFAIL(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pCmdAllocGraphics[i][0].ReleaseAndGetAddressOf())));
                SEEK_THROW_IFFAIL(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pCmdAllocGraphics[i][1].ReleaseAndGetAddressOf())));
            }
            SEEK_THROW_IFFAIL(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCmdAllocGraphics[0][0].Get(), nullptr, IID_PPV_ARGS(m_pCmdListGraphics[0].ReleaseAndGetAddressOf())));
            SEEK_THROW_IFFAIL(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCmdAllocGraphics[0][1].Get(), nullptr, IID_PPV_ARGS(m_pCmdListGraphics[1].ReleaseAndGetAddressOf())));
            m_pCmdListGraphics[1]->Close();

        }
        
        {
            // Init Compute Resources
            D3D12_COMMAND_QUEUE_DESC queue_desc = {};
            queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            m_pDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(m_pCommandQueueCompute.ReleaseAndGetAddressOf()));

            for (uint32_t i = 0; i < m_iBackBufferCount; i++)
            {
                SEEK_THROW_IFFAIL(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pCmdAllocCompute[i].ReleaseAndGetAddressOf())));
            }
            SEEK_THROW_IFFAIL(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCmdAllocCompute[0].Get(), nullptr, IID_PPV_ARGS(m_pCmdListCompute.ReleaseAndGetAddressOf())));
            m_pCmdListCompute->Close();

            m_pFenceCompute = m_pContext->RHIContextInstance().CreateFence();
        }

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

        return CheckCapabilitySetSupport();
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
SResult D3D12RHIContext::BeginFrame()
{
    return S_Success;
}
SResult D3D12RHIContext::EndFrame()
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