#pragma once
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_fence.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d_common/d3d_adapter.h"
#include "rhi/d3d_common/dxgi_helper.h"
#include "rhi/d3d12/d3d12_gpu_descriptor_allocator.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"

#include <thread>
#include <mutex>

SEEK_NAMESPACE_BEGIN

static uint32_t const NUM_BACK_BUFFERS = 3;
/******************************************************************************
* D3D12RHIContext
*******************************************************************************/
class D3D12RHIContext : public RHIContext, public DxgiHelper
{
public:
    D3D12RHIContext(Context* context);
    ~D3D12RHIContext();

    ID3D12Device* GetD3D12Device() { return m_pDevice.Get(); }
    ID3D12CommandQueue* GetD3D12CommandQueue() { return m_pCmdQueue.Get(); }
    ID3D12CommandAllocator* D3DRenderCmdAllocator() const;
    ID3D12GraphicsCommandList* D3DRenderCmdList() const;

    class PerThreadContext;
    PerThreadContext& CurThreadContext(bool is_render_context) const;
    void CommitRenderCmd(PerThreadContext& context);
    void SyncRenderCmd(PerThreadContext& context);
    void ResetRenderCmd(PerThreadContext& context);

    ID3D12CommandAllocator* D3DLoadCmdAllocator() const;
    ID3D12GraphicsCommandList* D3DLoadCmdList() const;
    void CommitLoadCmd();
    void SyncLoadCmd();
    void ResetLoadCmd();

    void ForceFlush();
    void ForceFinish();

    void RestoreRenderCmdStates(ID3D12GraphicsCommandList* cmd_list) {}


    D3D12GpuDescriptorBlock AllocRtvDescBlock(uint32_t size);
    void DeallocRtvDescBlock(D3D12GpuDescriptorBlock&& desc_block);
    void RenewRtvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size);
    D3D12GpuDescriptorBlock AllocDsvDescBlock(uint32_t size);
    void DeallocDsvDescBlock(D3D12GpuDescriptorBlock&& desc_block);
    void RenewDsvDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size);
    D3D12GpuDescriptorBlock AllocCbvSrvUavDescBlock(uint32_t size);
    void DeallocCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block);
    void RenewCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size);
    D3D12GpuDescriptorBlock AllocDynamicCbvSrvUavDescBlock(uint32_t size);
    void DeallocDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock&& desc_block);
    void RenewDynamicCbvSrvUavDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size);
    D3D12GpuDescriptorBlock AllocSamplerDescBlock(uint32_t size);
    void DeallocSamplerDescBlock(D3D12GpuDescriptorBlock&& desc_block);
    void RenewSamplerDescBlock(D3D12GpuDescriptorBlock& desc_block, uint32_t size);

    D3D12GpuMemoryBlock AllocUploadMemBlock(uint32_t size_in_bytes, uint32_t alignment);
    void DeallocUploadMemBlock(D3D12GpuMemoryBlock&& mem_block);
    void RenewUploadMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment);
    D3D12GpuMemoryBlock AllocReadbackMemBlock(uint32_t size_in_bytes, uint32_t alignment);
    void DeallocReadbackMemBlock(D3D12GpuMemoryBlock&& mem_block);
    void RenewReadbackMemBlock(D3D12GpuMemoryBlock& mem_block, uint32_t size_in_bytes, uint32_t alignment);

    std::vector<D3D12_RESOURCE_BARRIER>* FindResourceBarriers(ID3D12GraphicsCommandList* cmd_list, bool allow_creation);
    void FlushResourceBarriers(ID3D12GraphicsCommandList* cmd_list);
    void AddResourceBarrier(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_RESOURCE_BARRIER> barriers);
    void AddStallResource(ID3D12ResourcePtr const& resource);

    



    SResult                 Init() override;
    void                    Uninit() override;
    SResult                 CheckCapabilitySetSupport() override;    
    SResult                 AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;
    RHIMeshPtr              CreateMesh() override { return nullptr; }
    RHIShaderPtr            CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override { return nullptr; }
   
    RHITexturePtr           CreateTexture2D(ID3D12ResourcePtr const& tex) { return nullptr; }
    RHITexturePtr           CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override { return nullptr; }
    RHITexturePtr           CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override { return nullptr; }
    RHITexturePtr           CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data = {}) override { return nullptr; }

    RHIRenderBufferPtr      CreateConstantBuffer(uint32_t size, ResourceFlags flags) override { return nullptr; }
    RHIRenderBufferPtr      CreateStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) override { return nullptr; }
    RHIRenderBufferPtr      CreateRWStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) override { return nullptr; }
    RHIRenderBufferPtr      CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    RHIRenderBufferPtr      CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    RHIRenderBufferPtr      CreateVertexBuffer(uint32_t size, RHIRenderBufferData* pData) override { return nullptr; }
    RHIRenderBufferPtr      CreateIndexBuffer(uint32_t size, RHIRenderBufferData* pData) override { return nullptr; }

    RHIRenderTargetViewPtr  Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override { return nullptr; }
    RHIRenderTargetViewPtr  Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override { return nullptr; }
    RHIRenderTargetViewPtr  Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) override { return nullptr; }
    RHIDepthStencilViewPtr  Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override { return nullptr; }
    RHIDepthStencilViewPtr  Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override { return nullptr; }

    RHIFrameBufferPtr       CreateRHIFrameBuffer() override { return nullptr; }
    RHIProgramPtr           CreateRHIProgram() override { return nullptr; }
    RHITimeQueryPtr         CreateRHITimeQuery() override { return nullptr; }
    RHIFencePtr             CreateFence() override;

    SResult                 BeginFrame() override;
    SResult                 EndFrame() override;

    SResult                 BeginRenderPass(const RenderPassInfo& renderPassInfo) override { return 0; }
    SResult                 Render(RHIProgram* program, RHIMeshPtr const& mesh) override { return 0; }
    SResult                 EndRenderPass() override { return 0; }

    void                    BeginComputePass(const ComputePassInfo& computePassInfo) override { return ; }
    SResult                 Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) override { return 0; }
    SResult                 DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf) { return 0; }
    SResult                 DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type) { return 0; }
    SResult                 DrawInstanced(RHIProgram* program, RHIRenderStatePtr rs, MeshTopologyType type, uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override { return 0; }
    void                    EndComputePass() override { return ; }

    SResult                 SyncTexture(RHITexturePtr tex) override { return 0; }
    SResult                 CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) override { return 0; }
    SResult                 CopyTextureRegion(RHITexturePtr tex_src, RHITexturePtr tex_dst, int32_t dst_x = 0, int32_t dst_y = 0, int32_t dst_z = 0) override { return 0; }

    SResult                 BindContext() { return S_Success; }
    SResult                 DetachContext() { return S_Success; }

    void                    BeginCapture() override {}
    void                    EndCapture() override {}

    void                    BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name) override { return ; }
    void                    BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name) override { return ; }
    void                    BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name) override { return ; }
    void                    BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name) override { return ; }
    void                    BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name) override { return ; }
    void                    BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name) override { return ; }

protected:
    // Functions that only can been called by Context
    friend class Context;
    RHISamplerPtr           CreateSampler(SamplerDesc const& desc) override { return nullptr; }
    RHIRenderStatePtr       CreateRenderState(RenderStateDesc const& desc) override { return nullptr; }

    static const size_t         MAX_BACK_BUFFER_COUNT = 3;

private:
    uint32_t                        m_iBackBufferCount = 2;
    uint32_t                        m_iCurBufferIndex = 0;
    ID3D12DebugPtr                  m_pDebugCtrl = nullptr;

    ID3D12DevicePtr                 m_pDevice = nullptr;
    ID3D12CommandQueuePtr           m_pCmdQueue = nullptr;

/******************************************************************************
* D3D12RHIContext::PerThreadContext
*******************************************************************************/
    class PerThreadContext
    {
    public:
        PerThreadContext(ID3D12Device* d3d_device, RHIFencePtr const& frame_fence);
        ~PerThreadContext();

        void CommitCmd(ID3D12CommandQueue* d3d_cmd_queue, uint32_t frame_index);
        void SyncCmd(uint32_t frame_index);
        void ResetCmd(uint32_t frame_index);
        void Reset(uint32_t frame_index);

        std::thread::id ThreadID() const { return m_ThreadId; }
        ID3D12CommandAllocator* D3DCmdAllocator(uint32_t frame_index) const;
        ID3D12GraphicsCommandList* D3DCmdList() const;

        uint64_t FrameFenceValue(uint32_t frame_index) const;

    private:
        struct PerThreadPerFrameContext
        {
            ID3D12CommandAllocatorPtr d3d_cmd_allocator = nullptr;
            uint64_t fence_value = 0;
        };

    private:
        std::thread::id m_ThreadId;
        std::array<PerThreadPerFrameContext, NUM_BACK_BUFFERS> m_vPerFrameContexts;
        ID3D12GraphicsCommandListPtr m_pD3dCmdList;
        std::weak_ptr<RHIFence> m_pFrameFence;
    };
    mutable std::vector<std::unique_ptr<PerThreadContext>> m_vRenderThreadCmdContexts;
    mutable std::vector<std::unique_ptr<PerThreadContext>> m_vLoadThreadCmdContexts;

    struct PerFrameContext
    {
    public:
        PerFrameContext() = default;
        ~PerFrameContext();

        void AddStallResource(ID3D12ResourcePtr const& resource);
        void ClearStallResources();

    private:
        std::vector<ID3D12ResourcePtr> m_vStallResources;
        std::mutex m_Mutex;
    };
    std::array<PerFrameContext, NUM_BACK_BUFFERS> m_vPerFrameContexts;
    uint32_t m_iCurFrameIndex = 0;

    RHIFencePtr m_pFrameFence = nullptr;
    uint64_t m_iFrameFenceValue = 0;
    
        
    ID3D12CommandSignaturePtr       m_pDrawIndirectSign = nullptr;
    ID3D12CommandSignaturePtr       m_pDrawIndexedIndirectSign = nullptr;
    ID3D12CommandSignaturePtr       m_pDispatchIndirectSign = nullptr;

    std::vector<std::pair<ID3D12GraphicsCommandList*, std::vector<D3D12_RESOURCE_BARRIER>>> m_vResBarriers;
    std::vector<ID3D12ResourcePtr> m_vStallResources;
    std::mutex m_Mutex;

    D3D12GpuDescriptorAllocatorPtr m_pRtvDescAllocator = nullptr;
    D3D12GpuDescriptorAllocatorPtr m_pDsvDescAllocator = nullptr;
    D3D12GpuDescriptorAllocatorPtr m_pCbvSrvUavDescAllocator = nullptr;
    D3D12GpuDescriptorAllocatorPtr m_pDynamicCbvSrvUavDescAllocator = nullptr;
    D3D12GpuDescriptorAllocatorPtr m_pSamplerDescAllocator = nullptr;

    D3D12GpuDescriptorBlock     m_NullSrvUavDescBlock;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hNullSrvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hNullUavHandle;

    char const* shader_profiles_[(uint32_t)ShaderType::Num];

    D3D12GpuMemoryAllocatorPtr m_pUploadMemoryAllocator = nullptr;
    D3D12GpuMemoryAllocatorPtr m_pReadbackMemoryAllocator = nullptr;

    
};

SEEK_NAMESPACE_END