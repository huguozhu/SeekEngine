#pragma once
#include "rhi/base/rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"
#include "rhi/d3d_rhi_common/dxgi_helper.h"

SEEK_NAMESPACE_BEGIN

class D3D12RHIContext : public RHIContext, public DxgiHelper
{
public:
    D3D12RHIContext(Context* context);
    virtual SResult                 Init() override;
    virtual SResult                 CheckCapabilitySetSupport()override;
    virtual void                    Uninit() override;
    virtual SResult                 AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;

    virtual RHIMeshPtr              CreateMesh() override { return nullptr; }
    virtual RHIShaderPtr            CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override { return nullptr; }
    virtual std::vector<RHITexturePtr> CreateTexture2DFromNative(void* gpu_data, std::vector<PixelFormat> format) { return std::vector<RHITexturePtr>(); }

    virtual RHITexturePtr           CreateTexture2D(const RHITexture::Desc& tex_desc, const BitmapBufferPtr init_data = nullptr) override { return nullptr; }
    virtual RHITexturePtr           CreateTexture2D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) { return nullptr; }
    virtual RHITexturePtr           CreateTexture3D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas = {}) { return nullptr; }
    virtual RHITexturePtr           CreateTextureCube(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr>* init_data = nullptr) { return nullptr; }

    virtual RHIRenderBufferPtr      CreateEmptyVertexBuffer(uint32_t size, ResourceFlags flags) override { return nullptr; }
    virtual RHIRenderBufferPtr      CreateEmptyIndexBuffer(uint32_t size, ResourceFlags flags)override { return nullptr; }
    virtual RHITexturePtr           CreateEmptyTexture2D(RHITexture::Desc desc) override { return nullptr; }

    virtual RHIRenderBufferPtr      CreateConstantBuffer(uint32_t size, ResourceFlags flags) override { return nullptr; }
    virtual RHIRenderBufferPtr      CreateStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) override { return nullptr; }
    virtual RHIRenderBufferPtr      CreateRWStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) override { return nullptr; }
    virtual RHIRenderBufferPtr      CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    virtual RHIRenderBufferPtr      CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    virtual RHIRenderBufferPtr      CreateVertexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override { return nullptr; }
    virtual RHIRenderBufferPtr      CreateIndexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override { return nullptr; }
    virtual RHIRenderViewPtr        CreateRenderTargetView(RHITexturePtr const& tex, uint32_t lod = 0) override { return nullptr; }
    virtual RHIRenderViewPtr        CreateRenderTargetView(RHITexturePtr const& tex, CubeFaceType face, uint32_t lod = 0) { return nullptr; }
    virtual RHIRenderViewPtr        CreateDepthStencilView(RHITexturePtr const& tex) override { return nullptr; }
    virtual RHIRenderViewPtr        CreateDepthStencilView(RHITexturePtr const& tex, CubeFaceType face) { return nullptr; }
    virtual RHIFrameBufferPtr       CreateRHIFrameBuffer() override { return nullptr; }
    virtual RHIProgramPtr           CreateRHIProgram() override { return nullptr; }
    virtual RHITimeQueryPtr         CreateRHITimeQuery() override { return nullptr; }

    virtual void                    BindRHIProgram(RHIProgram* program) {}

    virtual SResult                 BeginFrame() override { return 0; }
    virtual SResult                 EndFrame() override { return 0; }

    virtual SResult                 BeginRenderPass(const RenderPassInfo& renderPassInfo) override { return 0; }
    virtual SResult                 Render(RHIProgram* program, RHIMeshPtr const& mesh) override { return 0; }
    virtual SResult                 EndRenderPass() override { return 0; }

    virtual void                    BeginComputePass(const ComputePassInfo& computePassInfo) override { return ; }
    virtual SResult                 Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) override { return 0; }
    virtual SResult                 DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf) { return 0; }
    virtual SResult                 DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type) { return 0; }
    virtual void                    EndComputePass() override { return ; }

    virtual SResult                 SyncTexture(RHITexturePtr tex) { return 0; }
    virtual SResult                 CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) { return 0; }

    virtual SResult                 BindContext() { return S_Success; }
    virtual SResult                 DetachContext() { return S_Success; }

    virtual void                    BeginCapture() {}
    virtual void                    EndCapture() {}

    //virtual void                    BeginRHITimerQuery(RHITimerQueryPtr&) override { return ; }
    //virtual void                    EndRHITimerQuery(RHITimerQueryPtr&) override { return ; }

    virtual void                    BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name) override { return ; }
    virtual void                    BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name) override { return ; }
    virtual void                    BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name) override { return ; }
    virtual void                    BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name) override { return ; }
    virtual void                    BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name) override { return ; }
    virtual void                    BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name) override { return ; }

    virtual RHIFencePtr             CreateFence() { return nullptr; }

protected:
    // Functions that only can been called by Context
    friend class Context;
    virtual RHISamplerPtr           CreateSampler(SamplerDesc const& desc) override { return nullptr; }
    virtual RHIRenderStatePtr       CreateRenderState(RenderStateDesc const& desc) override { return nullptr; }
    virtual const std::string GetName() const { return "D3D12RHIContext"; }

protected:
    ID3D12DebugPtr  m_pDebugCtrl = nullptr;
    ID3D12DevicePtr m_pDevice = nullptr;
};

SEEK_NAMESPACE_END