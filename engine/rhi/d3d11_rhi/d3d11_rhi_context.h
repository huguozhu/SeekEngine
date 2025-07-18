#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"
#include "rhi/d3d_rhi_common/dxgi_helper.h"
#include "rhi/d3d11_rhi/d3d11_query.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "utils/dll_loader.h"


SEEK_NAMESPACE_BEGIN
class D3D11Adapter;
class D3D11RHIContext : public RHIContext, public DxgiHelper
{
public:
    D3D11RHIContext(Context* context);
    virtual ~D3D11RHIContext();

    void SetD3D11Device(ID3D11Device* p);
    ID3D11Device* GetD3D11Device() { return m_pDevice.Get(); }
    void SetD3D11DeviceContext(ID3D11DeviceContext* p);
    ID3D11DeviceContext* GetD3D11DeviceContext() { return m_pDeviceContext.Get(); }

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
    ID3D11DeviceContextPtr m_pDeviceContext = nullptr;
    DXGI_SAMPLE_DESC m_msaa[CAP_MAX_TEXTURE_SAMPLE_COUNT + 1] = { {0, 0} };
    D3D11RHIFrameBuffer* m_pCurrentRHIFrameBuffer = nullptr;

// Rendering Object Factory
public:
    SResult             Init() override;
    void                Uninit() override;
    SResult             CheckCapabilitySetSupport() override;
    SResult             AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;
    RHIMeshPtr          CreateMesh() override;
    RHIShaderPtr        CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override;

    RHITexturePtr       CreateTexture2D(ID3D11Texture2DPtr const& tex);
    RHITexturePtr       CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override;
    RHITexturePtr       CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override;
    RHITexturePtr       CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data = {}) override;

    RHIRenderBufferPtr  CreateConstantBuffer(uint32_t size, ResourceFlags usage) override;
    RHIRenderBufferPtr  CreateStructuredBuffer  (uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RHIRenderBufferData* pData) override;
    RHIRenderBufferPtr  CreateRWStructuredBuffer(uint32_t size, ResourceFlags usage, uint32_t structure_byte_stride, RHIRenderBufferData* pData) override;
    RHIRenderBufferPtr  CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override;
    RHIRenderBufferPtr  CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) override;
    RHIRenderBufferPtr  CreateVertexBuffer(uint32_t size, RHIRenderBufferData* pData) override;
    RHIRenderBufferPtr  CreateIndexBuffer(uint32_t size, RHIRenderBufferData* pData) override;    

    RHIRenderTargetViewPtr Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override;
    RHIRenderTargetViewPtr Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override;
    RHIRenderTargetViewPtr Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) override;
    RHIDepthStencilViewPtr Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override;
    RHIDepthStencilViewPtr Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override;

    RHIFrameBufferPtr   CreateRHIFrameBuffer() override;
    RHIProgramPtr       CreateRHIProgram() override;
    RHITimeQueryPtr     CreateRHITimeQuery() override;
    RHIFencePtr         CreateFence() override;

    SResult             BeginFrame() override;
    SResult             EndFrame() override;

    SResult             BeginRenderPass(const RenderPassInfo& renderPassInfo) override;
    SResult             Render(RHIProgram* program, RHIMeshPtr const& mesh) override;
    SResult             EndRenderPass() override;

    void                BeginComputePass(const ComputePassInfo& computePassInfo) override;
    SResult             Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) override;
    SResult             DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf) override;
    SResult             DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type) override;
    SResult             DrawInstanced(RHIProgram* program, RHIRenderStatePtr rs, MeshTopologyType type, uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
    void                EndComputePass() override;

    SResult             SyncTexture(RHITexturePtr tex) override { return S_Success; }
    SResult             CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) override;
    SResult             CopyTextureRegion(RHITexturePtr tex_src, RHITexturePtr tex_dst, int32_t dst_x = 0, int32_t dst_y = 0, int32_t dst_z = 0) override;

    void                BeginCapture() override;
    void                EndCapture() override;

    void                BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name) override;
    void                BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name) override;
    void                BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name) override;
    void                BindTexture(ShaderType stage, uint32_t binding, const  RHITexture* texture, const char* name) override;
    void                BindRWTexture(ShaderType stage, uint32_t binding, const  RHITexture* rw_texture, const char* name) override;
    void                BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name) override;

    

protected:
    friend class Context;
    RHIRenderStatePtr   CreateRenderState(RenderStateDesc const& desc) override;
    RHISamplerPtr       CreateSampler(SamplerDesc const& desc) override;
};

SEEK_NAMESPACE_END
