#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_render_state.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_program.h"
#include "rhi/base/rhi_framebuffer.h"

#define SEEK_MACRO_FILE_UID 67     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

class RHIContext
{
public:
    struct RenderPassInfo
    {
        std::string name = "RenderPass";
        RHIFrameBuffer* fb = nullptr;
    };

    struct ComputePassInfo
    {
        std::string name = "ComputePass";
    };
    
    SResult                         BindRHIFrameBuffer(RHIFrameBufferPtr const& fb);
    RHIFrameBufferPtr const&        GetScreenRHIFrameBuffer() const;
    RHIFrameBufferPtr const&        GetFinalRHIFrameBuffer() const;
    RHIFrameBufferPtr const&        GetCurRHIFrameBuffer() const;
    void                            SetFinalRHIFrameBuffer(RHIFrameBufferPtr const& fb);

    CapabilitySet const&            GetCapabilitySet() const { return m_CapabilitySet; }
    RHIMeshPtr                      GetCubeMesh();
    RHIMeshPtr                      GetConeMesh();
    
    // if (D3D/OpenGL/Metal) VU's Y Direction Need Flipping
    virtual bool                    NeedFlipping() { return true; }

public: // virutal factory
    virtual ~RHIContext() {}
    virtual SResult                 Init() = 0;
    virtual SResult                 CheckCapabilitySetSupport() = 0;
    virtual void                    Uninit() = 0;
    virtual SResult                 AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) = 0;
    virtual RHIMeshPtr              CreateMesh() = 0;
    virtual RHIShaderPtr            CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) = 0;
    virtual std::vector<RHITexturePtr> CreateTexture2DFromNative(void* gpu_data, std::vector<PixelFormat> format) { return std::vector<RHITexturePtr>(); }
            RHITexturePtr           CreateTexture2D(const BitmapBufferPtr data);
    //virtual TexturePtr              CreateTexture(const Texture::Desc& tex_desc, const SubResourceData* init_data = nullptr, uint32_t init_data_size = 0) = 0;
    virtual RHITexturePtr           CreateTexture2D(const RHITexture::Desc& tex_desc, const BitmapBufferPtr init_data = nullptr) = 0;
    virtual RHITexturePtr           CreateTexture2D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas) { return nullptr; }
    virtual RHITexturePtr           CreateTexture3D(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr> init_datas = {}) { return nullptr; }
    virtual RHITexturePtr           CreateTextureCube(const RHITexture::Desc& tex_desc, std::vector<BitmapBufferPtr>* init_data = nullptr) { return nullptr; }
    
    virtual RHIRenderBufferPtr      CreateEmptyVertexBuffer(uint32_t size, ResourceFlags flags) = 0;
    virtual RHIRenderBufferPtr      CreateEmptyIndexBuffer(uint32_t size, ResourceFlags flags) = 0;
    virtual RHITexturePtr           CreateEmptyTexture2D(RHITexture::Desc desc) = 0;

    virtual RHIRenderBufferPtr      CreateConstantBuffer(uint32_t size, ResourceFlags flags) = 0;
    virtual RHIRenderBufferPtr      CreateStructuredBuffer  (uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) = 0;
    virtual RHIRenderBufferPtr      CreateRWStructuredBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride, RHIRenderBufferData* pData = nullptr) = 0;
    virtual RHIRenderBufferPtr      CreateByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    virtual RHIRenderBufferPtr      CreateRWByteAddressBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) { return nullptr; }
    virtual RHIRenderBufferPtr      CreateVertexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) = 0;
    virtual RHIRenderBufferPtr      CreateIndexBuffer(uint32_t size, ResourceFlags flags, RHIRenderBufferData* pData) = 0;
    virtual RHIRenderViewPtr        CreateRenderTargetView(RHITexturePtr const& tex, uint32_t lod = 0) = 0;
    virtual RHIRenderViewPtr        CreateRenderTargetView(RHITexturePtr const& tex, CubeFaceType face, uint32_t lod = 0) { return nullptr; }
    virtual RHIRenderViewPtr        CreateDepthStencilView(RHITexturePtr const& tex) = 0;
    virtual RHIRenderViewPtr        CreateDepthStencilView(RHITexturePtr const& tex, CubeFaceType face) { return nullptr; }
    virtual RHIFrameBufferPtr       CreateEmptyRHIFrameBuffer() = 0;
    virtual RHIProgramPtr           CreateRHIProgram() = 0;
    virtual RHITimerQueryPtr        CreateRHITimerQuery() = 0;

    virtual void                    BindRHIProgram(RHIProgram* program) {}

    virtual SResult                 BeginFrame() = 0;
    virtual SResult                 EndFrame() = 0;
    
    virtual SResult                 BeginRenderPass(const RenderPassInfo& renderPassInfo) = 0;
    virtual SResult                 Render(RHIProgram* program, RHIMeshPtr const& mesh) = 0;
    virtual SResult                 EndRenderPass() = 0;
    
    virtual void                    BeginComputePass(const ComputePassInfo& computePassInfo) = 0;
    virtual SResult                 Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) = 0;
    virtual SResult                 DispatchIndirect(RHIProgram* program, RHIRenderBufferPtr indirectBuf) { return 0; }
    virtual SResult                 DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIRenderBufferPtr indirectBuf, MeshTopologyType type) { return 0; }
    virtual void                    EndComputePass() = 0;

    virtual SResult                 SyncTexture(RHITexturePtr tex) { return ERR_NOT_IMPLEMENTED; }
    virtual SResult                 CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) { return ERR_NOT_IMPLEMENTED; }

    virtual SResult                 BindContext() { return S_Success; }
    virtual SResult                 DetachContext() { return S_Success; }

    virtual void                    BeginCapture() {}
    virtual void                    EndCapture() {}

    virtual void                    BeginRHITimerQuery(RHITimerQueryPtr&) = 0;
    virtual void                    EndRHITimerQuery(RHITimerQueryPtr&) = 0;

    // Global common reused variable
    RHISamplerPtr                   GetSampler(SamplerDesc const& desc);
    RHIRenderStatePtr               GetRenderState(RenderStateDesc const& desc);
    
    virtual void                    BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* cbuffer, const char* name) = 0;
    virtual void                    BindRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* buffer, const char* name) = 0;
    virtual void                    BindRWRHIRenderBuffer(ShaderType stage, uint32_t binding, const RHIRenderBuffer* rw_buffer, const char* name) = 0;
    virtual void                    BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name) = 0;
    virtual void                    BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name) = 0;
    virtual void                    BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name) = 0;

    virtual RHIFencePtr             CreateFence() { return nullptr; }

protected:
    // Functions that only can been called by Context
    friend class Context;
    virtual RHISamplerPtr           CreateSampler(SamplerDesc const& desc) = 0;
    virtual RHIRenderStatePtr       CreateRenderState(RenderStateDesc const& desc) = 0;


protected:
    RHIContext(Context* context);
    void                            CreateCommonMesh();

    Context*                        m_pContext;
    RHIFrameBufferPtr               m_pScreenRHIFrameBuffer = nullptr;
    RHIFrameBufferPtr               m_pFinalRHIFrameBuffer = nullptr;
    RHIFrameBufferPtr               m_pCurRHIFrameBuffer = nullptr;

    CapabilitySet                   m_CapabilitySet;
    RHIMeshPtr                      m_pCubeMesh;
    RHIMeshPtr                      m_pConeMesh;

    // Reused Variable
    std::map<SamplerDesc, RHISamplerPtr>           m_Samplers;
    std::map<RenderStateDesc, RHIRenderStatePtr>   m_RenderStates;
};

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
