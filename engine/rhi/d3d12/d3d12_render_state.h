#pragma once

#include "rhi/base/rhi_render_state.h"
#include "rhi/d3d12/d3d12_predeclare.h"

#include <variant>


SEEK_NAMESPACE_BEGIN

class D3D12RenderState : public RHIRenderState
{
public:
    D3D12RenderState(Context* context, RasterizerStateDesc const& rs_desc,
        DepthStencilStateDesc const& ds_desc, BlendStateDesc const& bs_desc);
    D3D12RenderState(Context* context, RenderStateDesc const& desc)
        : D3D12RenderState(context, desc.rasterizer, desc.depthStencil, desc.blend)
    {}
    SResult Active();

    ID3D12PipelineState* GetGraphicPso(RHIMesh& mesh, RHIShader& shader, RHIFrameBuffer& fb);

private:
    std::variant<D3D12_GRAPHICS_PIPELINE_STATE_DESC, D3D12_COMPUTE_PIPELINE_STATE_DESC> m_PsoDesc;
    mutable std::unordered_map<size_t, ID3D12PipelineStatePtr> m_vPsos;
};

class D3D12Sampler : public RHISampler
{
public:
    D3D12Sampler(Context* context, SamplerDesc const& desc);
    D3D12_SAMPLER_DESC const& GetD3DSampleDesc() const { return m_SampleDesc; }

private:
    D3D12_SAMPLER_DESC m_SampleDesc;
};

SEEK_NAMESPACE_END
