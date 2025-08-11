#pragma once

#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_context.h"


SEEK_NAMESPACE_BEGIN

class D3D12Resource
{
public:
    D3D12Resource(D3D12Context* rhi_context);
    ~D3D12Resource();

    ID3D12Resource* GetD3DResource() const { return m_pD3dResource.Get(); }
    uint32_t GetD3DResourceOffset() const { return m_iD3dResourceOffset; }
    void UpdateResourceBarrier(ID3D12GraphicsCommandList* cmd_list, uint32_t sub_res, D3D12_RESOURCE_STATES target_state);

protected:
    D3D12Context* m_pRHIContext = nullptr;
    ID3D12ResourcePtr m_pD3dResource;
    uint32_t m_iD3dResourceOffset;

    std::vector<D3D12_RESOURCE_STATES> m_vCurrStates;
};
using D3D12ResourcePtr = std::shared_ptr<D3D12Resource>;

SEEK_NAMESPACE_END
