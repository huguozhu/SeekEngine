﻿#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/render_definition.h"
#include "rhi/base/render_buffer.h"
#include "rhi/d3d11_rhi/d3d11_adapter.h"
#include "rhi/d3d11_rhi/d3d11_query.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "utils/dll_loader.h"
#include <DXProgrammableCapture.h>

SEEK_NAMESPACE_BEGIN
class D3D11Adapter;
class D3D11RHIContext : public RHIContext
{
public:
    D3D11RHIContext(Context* context);
    virtual ~D3D11RHIContext();

    D3D11AdapterPtr ActiveAdapter();
    void SetDXGIFactory1(IDXGIFactory1* p);
    IDXGIFactory1* GetDXGIFactory1() { return m_pDxgiFactory1.Get(); }
    void SetD3D11Device(ID3D11Device* p);
    ID3D11Device* GetD3D11Device() { return m_pDevice.Get(); }
    void SetD3D11DeviceContext(ID3D11DeviceContext* p);
    ID3D11DeviceContext* GetD3D11DeviceContext() { return m_pDeviceContext.Get(); }
    uint8_t GetDxgiSubVerion() { return m_iDxgiSubVer; }
    DXGI_SAMPLE_DESC GetDxgiSampleDesc(UINT sampleCount) { return m_msaa[sampleCount]; }

    void SetD3DRasterizerState(ID3D11RasterizerState* state);
    void SetD3DDepthStencilState(ID3D11DepthStencilState* state, uint16_t stencil_ref);
    void SetD3DBlendState(ID3D11BlendState* state, float4 blend_factor, uint32_t sample_mask);

    void SetD3DShader(ShaderType type, ID3D11DeviceChild* shader);
    void SetD3DShaderResourceViews(ShaderType type, uint32_t start_slot, uint32_t num_srvs, ID3D11ShaderResourceView* const* ppShaderResourceViews);
    void SetD3DSamplers(ShaderType type, uint32_t start_slot, uint32_t num_samplers, ID3D11SamplerState* const* ppSamplers);
    void SetD3DUnorderedAccessViews(ShaderType type, uint32_t start_slot, uint32_t num_uavs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews);
    void SetD3DConstantBuffers(ShaderType type, uint32_t start_slot, uint32_t num_cbuffers, ID3D11Buffer* const* ppCBuffers);

protected:
    ID3D11DevicePtr m_pDevice = nullptr;
    IDXGIFactory1Ptr m_pDxgiFactory1 = nullptr;
    IDXGIFactory2Ptr m_pDxgiFactory2 = nullptr;
    IDXGIFactory3Ptr m_pDxgiFactory3 = nullptr;
    IDXGIFactory4Ptr m_pDxgiFactory4 = nullptr;
    IDXGIFactory5Ptr m_pDxgiFactory5 = nullptr;
    IDXGIFactory6Ptr m_pDxgiFactory6 = nullptr;
    ID3D11DeviceContextPtr m_pDeviceContext = nullptr;

    std::vector<D3D11AdapterPtr> m_vAdapterList;
    uint32_t m_iCurAdapterNo = INVALID_ADAPTER_INDEX;
    uint8_t m_iDxgiSubVer = 0;

    // D3D11.0 has no dxgi debug, so we still need this
    //ComPtr<ID3D11InfoQueue> m_pD3D11InfoQueue;

    DXGI_SAMPLE_DESC m_msaa[CAP_MAX_TEXTURE_SAMPLE_COUNT + 1] = { {0, 0} };

    ComPtr<IDXGraphicsAnalysis> m_pGraphicsAnalysis = nullptr;

    D3D11TimerRHIQueryExecutor m_TimerRHIQueryExecutor{ m_pContext };

    D3D11RHIFrameBuffer* m_pCurrentRHIFrameBuffer = nullptr;

// Rendering Object Factory
public:
    virtual SResult Init() override;
    virtual void Uninit() override;
    virtual SResult CheckCapabilitySetSupport() override;
    virtual RHIMeshPtr CreateMesh() override;
    virtual RHIShaderPtr CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override;

    virtual RHITexturePtr       CreateTexture2D(ID3D11Texture2DPtr const& tex);
    virtual RHITexturePtr       CreateTexture2D(const  RHITexture::Desc& tex_desc, const BitmapBufferPtr init_data = nullptr) override;
    virtual RHITexturePtr       CreateTexture2D(const  RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) override;
    virtual RHITexturePtr       CreateTexture3D(const  RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) override;
    virtual RHITexturePtr       CreateTextureCube(const  RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr>* init_data = nullptr);
    virtual RHIRenderBufferPtr  CreateRHIRenderBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateConstantBuffer(uint32_t size, ResourceFlags usage) override;
    virtual RHIRenderBufferPtr  CreateStructuredBuffer  (uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateRWStructuredBuffer(uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateVertexBuffer(uint32_t size, ResourceFlags usage, RHIRenderBufferData* pData) override;
    virtual RHIRenderBufferPtr  CreateIndexBuffer(uint32_t size, ResourceFlags usage, RHIRenderBufferData* pData) override;

    virtual RenderStatePtr      CreateRenderState(RenderStateDesc const& desc) override;
    virtual SamplerPtr          CreateSampler(SamplerDesc const& desc) override;

    virtual RenderViewPtr       CreateRenderTargetView(RHITexturePtr const& tex, uint32_t lod = 0) override;
    virtual RenderViewPtr       CreateRenderTargetView(RHITexturePtr const& tex, CubeFaceType face, uint32_t lod = 0) override;
    virtual RenderViewPtr       CreateDepthStencilView(RHITexturePtr const& tex) override;
    virtual RenderViewPtr       CreateDepthStencilView(RHITexturePtr const& tex, CubeFaceType face) override;

    virtual RHIFrameBufferPtr   CreateRHIFrameBuffer() override;
    virtual RHIProgramPtr       CreateRHIProgram() override;
    virtual TimerRHIQueryPtr    CreateTimerRHIQuery() override;

    virtual SResult             AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;

    virtual SResult             BeginFrame() override;
    virtual SResult             EndFrame() override;

    virtual SResult             BeginRenderPass(const RenderPassInfo& renderPassInfo) override;
    virtual SResult             Render(RHIProgram* program, RHIMeshPtr const& mesh) override;
    virtual SResult             EndRenderPass() override;

    virtual void                BeginComputePass(const ComputePassInfo& computePassInfo) override;
    virtual SResult             Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) override;
    virtual SResult             DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf) override;
    virtual SResult             DrawIndirect(RHIProgram* program, RenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type) override;
    virtual void                EndComputePass() override;

    virtual SResult SyncTexture(RHITexturePtr tex) override { return S_Success; }
    virtual SResult CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) override;

    virtual void BeginCapture() override;
    virtual void EndCapture() override;

    virtual void BeginTimerRHIQuery(TimerRHIQueryPtr& timerRHIQuery) override;
    virtual void EndTimerRHIQuery(TimerRHIQueryPtr& timerRHIQuery) override;

    virtual void BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name) override;
    virtual void BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name) override;
    virtual void BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name) override;
    virtual void BindTexture(ShaderType stage, uint32_t binding, const  RHITexture* texture, const char* name) override;
    virtual void BindRWTexture(ShaderType stage, uint32_t binding, const  RHITexture* rw_texture, const char* name) override;
    virtual void BindSampler(ShaderType stage, uint32_t binding, const Sampler* sampler, const char* name) override;
};

SEEK_NAMESPACE_END