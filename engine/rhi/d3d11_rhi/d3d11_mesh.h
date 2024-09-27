#pragma once
#include <vector>
#include <tuple>
#include "rhi/base/rhi_mesh.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3D11Mesh : public RHIMesh
{
public:
    D3D11Mesh(Context* context);
    virtual ~D3D11Mesh() override;

    SResult Active(RHIProgram* program);
    SResult Deactive() const;
    ID3D11InputLayout* GetInputLayout(RHIProgram* program) const;

private:
    mutable std::vector<D3D11_INPUT_ELEMENT_DESC> m_vD3DInputElementDescs;
    mutable std::tuple<std::vector<ID3D11Buffer*>, std::vector<UINT>, std::vector<UINT>> m_vD3DInputElementInfos;   // Buffer-stride-offset
    mutable std::vector<std::pair<RHIProgram*, ID3D11InputLayoutPtr>> m_vD3DInputLayouts;
};


SEEK_NAMESPACE_END
