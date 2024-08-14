#pragma once

#include "kernel/kernel.h"
#include "rhi/rhi_context.h"
#include "rhi/render_definition.h"
#include "rhi/render_buffer.h"
#include "d3d11_rhi/d3d11_adapter.h"
#include "d3d11_rhi/d3d11_query.h"
#include "d3d11_rhi/d3d11_framebuffer.h"
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

    D3D11TimerQueryExecutor m_TimerQueryExecutor{ m_pContext };

    D3D11FrameBuffer* m_pCurrentFrameBuffer = nullptr;

// Rendering Object Factory
public:
    virtual SResult Init() override;
    virtual void Uninit() override;
    virtual SResult CheckCapabilitySetSupport() override;
    virtual MeshPtr CreateMesh() override;
    virtual ShaderPtr CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override;

    virtual TexturePtr      CreateTexture2D(ID3D11Texture2DPtr const& tex);
    virtual TexturePtr      CreateTexture2D(const Texture::Desc& tex_desc, const BitmapBufferPtr init_data = nullptr) override;
    virtual TexturePtr      CreateTexture2D(const Texture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) override;
    virtual TexturePtr      CreateTexture3D(const Texture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) override;
    virtual TexturePtr      CreateTextureCube(const Texture::Desc& tex_desc, std::vector<BitmapBufferPtr>* init_data = nullptr);
    virtual RenderBufferPtr CreateRenderBuffer(uint32_t size, ResourceFlags flags, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateConstantBuffer(uint32_t size, ResourceFlags usage) override;
    virtual RenderBufferPtr CreateStructuredBuffer  (uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateRWStructuredBuffer(uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateVertexBuffer(uint32_t size, ResourceFlags usage, RenderBufferData* pData) override;
    virtual RenderBufferPtr CreateIndexBuffer(uint32_t size, ResourceFlags usage, RenderBufferData* pData) override;

    virtual RenderStatePtr  CreateRenderState(RenderStateDesc const& desc) override;
    virtual SamplerPtr      CreateSampler(SamplerDesc const& desc) override;

    virtual RenderViewPtr   CreateRenderTargetView(TexturePtr const& tex, uint32_t lod = 0) override;
    virtual RenderViewPtr   CreateRenderTargetView(TexturePtr const& tex, CubeFaceType face, uint32_t lod = 0) override;
    virtual RenderViewPtr   CreateDepthStencilView(TexturePtr const& tex) override;
    virtual RenderViewPtr   CreateDepthStencilView(TexturePtr const& tex, CubeFaceType face) override;

    virtual FrameBufferPtr  CreateFrameBuffer() override;
    virtual ProgramPtr      CreateProgram() override;
    virtual TimerQueryPtr   CreateTimerQuery() override;

    virtual SResult AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;

    virtual SResult       BeginFrame() override;
    virtual SResult       EndFrame() override;

    virtual SResult       BeginRenderPass(const RenderPassInfo& renderPassInfo) override;
    virtual SResult       Render(Program* program, MeshPtr const& mesh) override;
    virtual SResult       EndRenderPass() override;

    virtual void            BeginComputePass(const ComputePassInfo& computePassInfo) override;
    virtual SResult       Dispatch(Program* program, uint32_t x, uint32_t y, uint32_t z) override;
    virtual SResult       DispatchIndirect(Program* program, RenderBufferPtr indirectBuf) override;
    virtual SResult       DrawIndirect(Program* program, RenderStatePtr rs, RenderBufferPtr indirectBuf, MeshTopologyType type) override;
    virtual void            EndComputePass() override;

    virtual SResult SyncTexture(TexturePtr tex) override { return S_Success; }
    virtual SResult CopyTexture(TexturePtr tex_src, TexturePtr tex_dst) override;

    virtual void BeginCapture() override;
    virtual void EndCapture() override;

    virtual void BeginTimerQuery(TimerQueryPtr& timerQuery) override;
    virtual void EndTimerQuery(TimerQueryPtr& timerQuery) override;

    virtual void BindConstantBuffer(ShaderType stage, uint32_t binding, const RenderBuffer* cbuffer, const char* name) override;
    virtual void BindRenderBuffer(ShaderType stage, uint32_t binding, const RenderBuffer* buffer, const char* name) override;
    virtual void BindRWRenderBuffer(ShaderType stage, uint32_t binding, const RenderBuffer* rw_buffer, const char* name) override;
    virtual void BindTexture(ShaderType stage, uint32_t binding, const Texture* texture, const char* name) override;
    virtual void BindRWTexture(ShaderType stage, uint32_t binding, const Texture* rw_texture, const char* name) override;
    virtual void BindSampler(ShaderType stage, uint32_t binding, const Sampler* sampler, const char* name) override;
};

SEEK_NAMESPACE_END
