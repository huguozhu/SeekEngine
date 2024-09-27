#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "rhi/base/texture.h"
#include "rhi/base/viewport.h"

SEEK_NAMESPACE_BEGIN

class FrameBuffer
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

//    int32_t                             Left()      const { return m_iLeft; }
//    int32_t                             Top()       const { return m_iTop; }
//    uint32_t                            Width()     const { return m_iWidth; }
//    uint32_t                            Height()    const { return m_iHeight; }
    bool                                IsDirty()   const { return m_bViewDirty; }
    void                                SetDirty(bool b)  { m_bViewDirty = b; }

    void                                SetViewport(Viewport viewport) { m_stViewport = viewport; }
    Viewport const &                    GetViewport();

    void                                AttachTargetView(Attachment att, RenderViewPtr const& view);
    void                                AttachDepthStencilView(RenderViewPtr const& view);

    RenderViewPtr                       GetRenderTarget(Attachment att) const;
    std::array<RenderViewPtr, MAX_COLOR_ATTACHMENTS> const& GetRenderTargets() const { return m_vRenderTargets; }
    RenderViewPtr              const&   GetDepthStencilView() const { return m_pDepthStencilView; }
    
    virtual SResult                   OnBind() = 0;
    virtual SResult                   OnUnbind() = 0;
    virtual SResult                   SwapBuffers() { return S_Success; }
    
    SResult Bind();
    void Unbind();
    
    void SetColorLoadOption(Attachment attachment, LoadOption loadOption);
    void SetColorStoreOption(Attachment attachment, StoreOption storeOption);
    
    void SetDepthLoadOption(LoadOption loadOption);
    void SetDepthStoreOption(StoreOption storeOption);
    
    void Reset();
    
    void SetSampleNum(uint32_t sampleNum)
    {
        m_sampleNum = sampleNum;
    }
    
    uint32_t GetSampleNum()
    {
        return m_sampleNum;
    }
    
protected:
    FrameBuffer(Context* context) :m_pContext(context) {}
    virtual ~FrameBuffer() {}

    Context*                            m_pContext = nullptr;

    bool                                m_bViewDirty = false;
    Viewport                            m_stViewport;
    std::array<RenderViewPtr, MAX_COLOR_ATTACHMENTS> m_vRenderTargets;
    RenderViewPtr                       m_pDepthStencilView;
    uint32_t                            m_sampleNum = 1;
    
    LoadOption m_colorLoadOptions[MAX_COLOR_ATTACHMENTS];
    StoreOption m_colorStoreOptions[MAX_COLOR_ATTACHMENTS];
    
    LoadOption m_depthLoadOption;
    StoreOption m_depthStoreOption;
    
    std::array<TexturePtr, MAX_COLOR_ATTACHMENTS> m_msaaColorTex;
    TexturePtr m_msaaDepthStencilTex = nullptr;
    
    uint32_t m_resolveFlag = RESOLVE_NONE;
};

SEEK_NAMESPACE_END
