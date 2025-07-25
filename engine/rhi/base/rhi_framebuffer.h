#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/viewport.h"

SEEK_NAMESPACE_BEGIN

class RHIFrameBuffer
{
public:
    enum Attachment: uint8_t
    {
        Color0  = 0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6,
        Color7,

        Depth,
        Stencil,
    };
    static const uint8_t MAX_COLOR_ATTACHMENTS = 8;
    static const uint8_t ATTACHMENTS_NUM = to_underlying(Attachment::Stencil);
    
    static constexpr uint32_t RESOLVE_NONE = 0;
    static inline uint32_t RequireResolve(Attachment attachment)
    {
        return 1 << to_underlying(attachment);
    }
    
    static inline bool NeedResolve(uint32_t resolveFlag, Attachment attachment)
    {
        return (RequireResolve(attachment) & resolveFlag) != 0;
    }
    using RtvArray = std::array<RHIRenderTargetViewPtr, MAX_COLOR_ATTACHMENTS>;
    enum class LoadAction
    {
        Load = 0,
        DontCare,
        Clear,
    };
    
    enum class StoreAction
    {
        Store = 0,
        DontCare,
    };
    
    struct LoadOption
    {
        LoadOption()
            : loadAction(LoadAction::DontCare)
            , clearColor({ 1.0, 1.0, 1.0, 1.0})
        { }

        LoadOption(LoadAction _loadAction)
            : loadAction(_loadAction)
            , clearColor({ 1.0, 1.0, 1.0, 1.0 })
        { }
        
        LoadOption(float4 colorClear)
            : loadAction(LoadAction::Clear)
            , clearColor(colorClear)
        { }
        
        LoadOption(float depthClear)
            : loadAction(LoadAction::Clear)
            , clearDepth(depthClear)
        { }
        
        LoadOption(int32_t stencilClear)
            : loadAction(LoadAction::Clear)
            , clearStencil(stencilClear)
        { }
        
        LoadOption(const LoadOption& rhs)
            : loadAction(rhs.loadAction)
            , clearColor(rhs.clearColor)
        { }
    
        LoadOption& operator=(const LoadOption& rhs)
        {
            loadAction = rhs.loadAction;
            clearColor = rhs.clearColor;
            return *this;
        }
        
        LoadAction loadAction;
        union {
            float4 clearColor;
            float clearDepth;
            int32_t clearStencil;
        }; // clear value used when loadAction is LoadAction::Clear
    };
    
    struct StoreOption
    {
        StoreOption()
            : storeAction(StoreAction::Store)
        { }        
        StoreOption(StoreAction _storeAction)
            : storeAction(_storeAction)
        { }        
        StoreAction storeAction = StoreAction::Store;
    };

    bool                        IsDirty()   const { return m_bViewDirty; }
    void                        SetDirty(bool b)  { m_bViewDirty = b; }

    void                        SetViewport(Viewport viewport) { m_stViewport = viewport; }
    Viewport const &            GetViewport();

    void                        AttachTargetView(Attachment att, RHIRenderTargetViewPtr const& view);
    void                        DetachTargetView(Attachment att);
    void                        DetachAllTargetView();

    void                        AttachDepthStencilView(RHIDepthStencilViewPtr const& view);
    void                        DetachDepthStencilView();

    RHIRenderTargetViewPtr      GetRenderTarget(Attachment att) const;
    RtvArray const&             GetRenderTargets() const { return m_vRenderTargets; }
    RHIDepthStencilViewPtr const& GetDepthStencilView() const { return m_pDepthStencilView; }

    enum ClearBufferMask : uint32_t
    {
        CBM_Color = 0x01,
        CBM_Depth = 0x02,
        CBM_Stencil = 0x04,
        CBM_ALL = CBM_Color | CBM_Depth | CBM_Stencil,
    };
    virtual void Clear(uint32_t flags = CBM_ALL, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0), float depth = 1.0, int32_t stencil = 0) {}
    virtual void ClearRenderTarget(Attachment att, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0)) {}
    
    virtual SResult             OnBind() = 0;
    virtual SResult             OnUnbind() = 0;
    virtual SResult             SwapBuffers() { return S_Success; }
    virtual SResult             CopyRenderTarget(Attachment attachment, const RHITexturePtr& texture);
    virtual RHITexture::Desc    GetRenderTargetDesc(Attachment attachment);
    
    SResult Bind();
    void    Unbind();
    
    void SetColorLoadOption(Attachment attachment, LoadOption loadOption);
    void SetColorStoreOption(Attachment attachment, StoreOption storeOption);    
    void SetDepthLoadOption(LoadOption loadOption);
    void SetDepthStoreOption(StoreOption storeOption);
    
    void Reset();    
    void SetSampleNum(uint32_t sampleNum)  { m_sampleNum = sampleNum; }
    uint32_t GetSampleNum() { return m_sampleNum; }
    
protected:
    RHIFrameBuffer(Context* context) :m_pContext(context) {}
    virtual ~RHIFrameBuffer() {}

    Context*                m_pContext = nullptr;
    bool                    m_bViewDirty = false;
    Viewport                m_stViewport;
    RtvArray                m_vRenderTargets;
    RHIDepthStencilViewPtr  m_pDepthStencilView;
    uint32_t                m_sampleNum = 1;
    
    LoadOption m_colorLoadOptions[MAX_COLOR_ATTACHMENTS];
    StoreOption m_colorStoreOptions[MAX_COLOR_ATTACHMENTS];
    
    LoadOption m_depthLoadOption;
    StoreOption m_depthStoreOption;
    
    std::array<RHITexturePtr, MAX_COLOR_ATTACHMENTS> m_msaaColorTex;
    RHITexturePtr m_msaaDepthStencilTex = nullptr;
    
    uint32_t m_resolveFlag = RESOLVE_NONE;
};

SEEK_NAMESPACE_END
