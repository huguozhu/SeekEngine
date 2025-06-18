#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_definition.h"
#include "math/vector.h"
#include "math/color.h"
#include "math/math_utility.h"

SEEK_NAMESPACE_BEGIN

//undef to fix the conflit with X11
#undef None
#undef Always

/******************************************************************************
* RenderState
*******************************************************************************/
enum class CullMode : uint8_t
{
    None = 0,
    Front,
    Back,
};

enum class FillMode : uint8_t
{
    Point = 0,
    Wireframe,
    Solid,
};

struct RasterizerStateDesc
{
    bool            bScissorEnable = false;
    bool            bFrontFaceCCW = false;
    bool            bDepthClip = true;
    CullMode        eCullMode = CullMode::None;
    FillMode        eFillMode = FillMode::Solid;
    float           fLineWidth = 1.0;
};

enum class CompareFunction : uint8_t
{
    Less = 0,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    Never,
    Always,
};

enum class StencilOperation : uint8_t
{
    Keep = 0,
    Zero,
    Replace,
    Increment,
    Decrement,
    Invert,
    SaturatedIncrement,
    SaturatedDecrement,
};

struct DepthStencilStateDesc
{
    bool                    bDepthEnable = true;
    bool                    bDepthWriteMask = true;
    CompareFunction         eDepthFunc = CompareFunction::Less;

    bool                    bFrontStencilEnable = false;
    CompareFunction         eFrontStencilFunction = CompareFunction::Always;
    StencilOperation        eFrontStencilFail = StencilOperation::Keep;
    StencilOperation        eFrontStencilDepthFail = StencilOperation::Keep;
    StencilOperation        eFrontStencilPass = StencilOperation::Keep;
    uint16_t                iFrontStencilRef = 0;
    uint16_t                iFrontStencilReadMask = 0xffff;
    uint16_t                iFrontStencilWriteMask = 0xffff;

    bool                    bBackStencilEnable = false;
    CompareFunction         eBackStencilFunction = CompareFunction::Always;
    StencilOperation        eBackStencilFail = StencilOperation::Keep;
    StencilOperation        eBackStencilDepthFail = StencilOperation::Keep;
    StencilOperation        eBackStencilPass = StencilOperation::Keep;
    uint16_t                iBackStencilRef = 0;
    uint16_t                iBackStencilReadMask = 0xffff;
    uint16_t                iBackStencilWriteMask = 0xffff;
};

enum class BlendOperation : uint8_t
{
    Add = 0,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

enum class BlendFactor : uint8_t
{
    Zero = 0,
    One,

    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,

    DstColor,
    InvDstColor,
    DstAlpha,
    InvDstAlpha,

    SrcAlphaSat,
    BlendFactor,
    InvBlendFactor,

    Src1Color,
    InvSrc1Color,
    Src1Alpha,
    InvSrc1Alpha,
};

enum ColorWriteMask : uint8_t
{
    CWM_Red     = 0x01,
    CWM_Green   = 0x02,
    CWM_Blue    = 0x04,
    CWM_Alpha   = 0x08,

    CWM_None    = 0x00,
    CWM_RG      = CWM_Red | CWM_Green,
    CWM_RGB     = CWM_Red | CWM_Green | CWM_Blue,
    CWM_RGBA    = CWM_Red | CWM_Green | CWM_Blue | CWM_Alpha,
};

struct BlendStateDesc
{
    float4          fBlendFactor = float4(1.0);
    uint32_t        iSampleMask = 0xffffffff;

    bool            bIndependentBlendEnable = false;
    bool            bAlphaToCoverageEnable = false;

    struct TargetBlendDesc
    {
        bool            bBlendEnable    = false;
        BlendFactor     eSrcBlendColor  = BlendFactor::One;
        BlendFactor     eDstBlendColor  = BlendFactor::Zero;
        BlendOperation  eBlendOpColor   = BlendOperation::Add;

        BlendFactor     eSrcBlendAlpha  = BlendFactor::One;
        BlendFactor     eDstBlendAlpha  = BlendFactor::Zero;
        BlendOperation  eBlendOpAlpha   = BlendOperation::Add;
        ColorWriteMask  bColorWriteMask = CWM_RGBA;
    };
    std::array<TargetBlendDesc, 8> stTargetBlend;
};

struct RenderStateDesc
{
    RasterizerStateDesc     rasterizer = {};
    DepthStencilStateDesc   depthStencil = {};
    BlendStateDesc          blend = {};

    static const RenderStateDesc& Default3D();
    static const RenderStateDesc& Default2D();
    static const RenderStateDesc& PostProcess();
    static const RenderStateDesc& PostProcessAccumulate();
    static const RenderStateDesc& Skybox();
    static const RenderStateDesc& Particle();
    static const RenderStateDesc& WaterMark();

	static const RenderStateDesc& GBuffer();
    static const RenderStateDesc& ShadowCopyR();
    static const RenderStateDesc& ShadowCopyG();
    static const RenderStateDesc& ShadowCopyB();
    static const RenderStateDesc& ShadowCopyA();
    static const RenderStateDesc& Lighting();

    size_t   Hash() const;
    bool     operator==(RenderStateDesc const& rhs) const;
    bool     operator<(RenderStateDesc const& rhs) const;
};

class RHIRenderState
{
public:
    virtual ~RHIRenderState(){}

    RenderStateDesc const&      GetRenderStateDesc() const { return m_stRenderStateDesc; }

    virtual bool                IsTransparent() { return m_stRenderStateDesc.blend.stTargetBlend[0].bBlendEnable; }

protected:
    RHIRenderState(Context* context)
        : m_pContext(context)
    {}
    RHIRenderState(Context* context, RenderStateDesc const& desc)
        : m_pContext(context), m_stRenderStateDesc(desc)
    {}
    RHIRenderState(Context* context, RasterizerStateDesc const& rs_desc, DepthStencilStateDesc const& ds_desc, BlendStateDesc const& bs_desc)
        : m_pContext(context)
    {
        m_stRenderStateDesc.rasterizer = rs_desc;
        m_stRenderStateDesc.depthStencil = ds_desc;
        m_stRenderStateDesc.blend = bs_desc;
    }

    Context*                m_pContext = nullptr;
    RenderStateDesc         m_stRenderStateDesc = {};
};

/******************************************************************************
* RHISampler
*******************************************************************************/
enum class TexFilterOp : uint8_t
{
    Min_Mag_Mip_Point,
    Min_Mag_Point_Mip_Linear,
    Min_Point_Mag_Linear_Mip_Point,
    Min_Point_Mag_Mip_Linear,
    Min_Linear_Mag_Mip_Point,
    Min_Linear_Mag_Point_Mip_Linear,
    Min_Mag_Linear_Mip_Point,
    Min_Mag_Mip_Linear,
    Anisotropic,
};

enum class TexAddressMode : uint8_t
{
    Wrap,
    Clamp,
    Mirror,
    Border,
};

struct SamplerDesc
{
    TexFilterOp     eFilterOp = TexFilterOp::Min_Mag_Mip_Linear;
    TexAddressMode  eAddrModeU = TexAddressMode::Wrap;
    TexAddressMode  eAddrModeV = TexAddressMode::Wrap;
    TexAddressMode  eAddrModeW = TexAddressMode::Wrap;
    CompareFunction eCompareFun = CompareFunction::Never;
    Color           cBoarderColor = Color(0, 0, 0, 0);
    uint32_t        iMaxAnisotropy = 4;
    float           iMinLod = 0;
    float           iMaxLod = Math::FLOAT_MAX;
    float           iMipMapLodBias = 0.0f;

    static SamplerDesc GetSamplerDescByName(std::string const& name);
    static SamplerDesc PointSampler();
    static SamplerDesc LinearSampler();
    static SamplerDesc BilinearSampler();
    static SamplerDesc PrefilterMapSampler();
    static SamplerDesc AnisotropicSampler();
    static SamplerDesc SkyboxSampler();
    static SamplerDesc ShadowMapSampler();
    static SamplerDesc NoiseSampler();

    size_t   Hash() const;
    bool     operator==(SamplerDesc const& rhs) const;
    bool     operator<(SamplerDesc const& rhs) const;
};

class RHISampler
{
protected:
    RHISampler(Context* context, SamplerDesc const& desc)
        : m_pContext(context), m_stSamplerDesc(desc)
    {}
    virtual ~RHISampler() {}

protected:
    Context*    m_pContext = nullptr;
    SamplerDesc m_stSamplerDesc = {};
};

SEEK_NAMESPACE_END
