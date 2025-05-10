#include "rhi/base/rhi_render_state.h"
#include "math/hash.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 38     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// SamplerDesc Declare
//////////////////////////////////////////////////////////////////////////
SamplerDesc SamplerDesc::GetSamplerDescByName(std::string const& name)
{
    if      (name == "point_sampler")           return PointSampler();
    else if (name == "linear_sampler")          return LinearSampler();
    else if (name == "bilinear_sampler")        return BilinearSampler();
    else if (name == "anisotropic_sampler")     return AnisotropicSampler();
    else if (name == "skybox_sampler")          return SkyboxSampler();
    else
    {
        LOG_ERROR("Can not find %s SamplerDesc, return default Desc", name.c_str());
        return SamplerDesc();
    }
}
SamplerDesc SamplerDesc::PointSampler()
{
    static SamplerDesc desc;
    desc.eFilterOp = TexFilterOp::Min_Mag_Mip_Point;
    desc.eAddrModeU = TexAddressMode::Clamp;
    desc.eAddrModeV = TexAddressMode::Clamp;
    desc.eCompareFun = CompareFunction::Always;
    return desc;
}
SamplerDesc SamplerDesc::LinearSampler()
{
    static SamplerDesc desc;
    desc.eCompareFun = CompareFunction::Always;
    return desc;
}
SamplerDesc SamplerDesc::BilinearSampler()
{
    static SamplerDesc desc;
    desc.eFilterOp = TexFilterOp::Min_Mag_Linear_Mip_Point;
    return desc;
}
SamplerDesc SamplerDesc::AnisotropicSampler()
{
    static SamplerDesc desc;
    desc.eFilterOp = TexFilterOp::Anisotropic;
    desc.eAddrModeU = TexAddressMode::Clamp;
    desc.eAddrModeV = TexAddressMode::Clamp;
    return desc;
}
SamplerDesc SamplerDesc::SkyboxSampler()
{
    static SamplerDesc desc;
    desc.eAddrModeU = TexAddressMode::Clamp;
    desc.eAddrModeV = TexAddressMode::Clamp;
    desc.eAddrModeW = TexAddressMode::Clamp;
    return desc;
}

//////////////////////////////////////////////////////////////////////////
// RenderStateDesc Declare
//////////////////////////////////////////////////////////////////////////
const RenderStateDesc& RenderStateDesc::Default3D()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = true;
    return desc;
}
const RenderStateDesc& RenderStateDesc::Default2D()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    for (auto& rb : desc.blend.stTargetBlend) rb.bBlendEnable = false;
    return desc;
}
const RenderStateDesc& RenderStateDesc::PostProcess()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    return desc;
}
const RenderStateDesc& RenderStateDesc::Skybox()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = true;
    desc.depthStencil.bDepthWriteMask = false;
    desc.depthStencil.eDepthFunc = CompareFunction::Equal;
    return desc;
}
const RenderStateDesc& RenderStateDesc::Particle()
{
    static RenderStateDesc desc;
    desc.rasterizer.eCullMode = CullMode::None;
    desc.depthStencil.bDepthEnable = true;
    desc.depthStencil.bDepthWriteMask = false;

    desc.blend.bIndependentBlendEnable = false;
    for (auto& rb : desc.blend.stTargetBlend)
    {
        rb.bBlendEnable = true;
        rb.eSrcBlendColor = BlendFactor::SrcAlpha;
        rb.eDstBlendColor = BlendFactor::InvSrcAlpha;

        rb.eSrcBlendAlpha = BlendFactor::DstAlpha;
        rb.eDstBlendAlpha = BlendFactor::One;
    }    
    return desc;
}
const RenderStateDesc& RenderStateDesc::WaterMark()
{
    static RenderStateDesc desc;
    desc.rasterizer.eCullMode = CullMode::None;
    desc.depthStencil.bDepthEnable = false;
    desc.depthStencil.bDepthWriteMask = false;

    desc.blend.bIndependentBlendEnable = false;
    for (auto& rb : desc.blend.stTargetBlend)
    {
        rb.bBlendEnable = true;
        rb.eSrcBlendColor = BlendFactor::SrcAlpha;
        rb.eDstBlendColor = BlendFactor::InvSrcAlpha;

        rb.eSrcBlendAlpha = BlendFactor::DstAlpha;
        rb.eDstBlendAlpha = BlendFactor::One;
    }
    return desc;
}
const RenderStateDesc& RenderStateDesc::ShadowCopyR()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    for (auto& rb : desc.blend.stTargetBlend) rb.bColorWriteMask = ColorWriteMask::CWM_Red;
    return desc;
}
const RenderStateDesc& RenderStateDesc::ShadowCopyG()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    for (auto& rb : desc.blend.stTargetBlend) rb.bColorWriteMask = ColorWriteMask::CWM_Green;
    return desc;
}
const RenderStateDesc& RenderStateDesc::ShadowCopyB()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    for (auto& rb : desc.blend.stTargetBlend) rb.bColorWriteMask = ColorWriteMask::CWM_Blue;
    return desc;
}
const RenderStateDesc& RenderStateDesc::ShadowCopyA()
{
    static RenderStateDesc desc;
    desc.depthStencil.bDepthEnable = false;
    for (auto& rb : desc.blend.stTargetBlend) rb.bColorWriteMask = ColorWriteMask::CWM_Alpha;
    return desc;
}
//////////////////////////////////////////////////////////////////////////

size_t RenderStateDesc::Hash() const
{
    STRUCT_HASH();
}
bool RenderStateDesc::operator==(RenderStateDesc const& rhs) const
{
    return this->Hash() == rhs.Hash();
}
bool RenderStateDesc::operator<(RenderStateDesc const& rhs) const
{
    return this->Hash() < rhs.Hash();
}

size_t SamplerDesc::Hash() const
{
    STRUCT_HASH();
}
bool SamplerDesc::operator==(SamplerDesc const& rhs) const
{
    return this->Hash() == rhs.Hash();
}
bool SamplerDesc::operator<(SamplerDesc const& rhs) const
{
    return this->Hash() < rhs.Hash();
}

SEEK_NAMESPACE_END


#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
