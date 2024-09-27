#pragma once
#include <vector>
#include <tuple>
#include "rhi/base/mesh.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3D11Mesh : public Mesh
{
public:
    D3D11Mesh(Context* context);
    virtual ~D3D11Mesh() override;

    SResult Active(Program* program);
    SResult Deactive() const;
    ID3D11InputLayout* GetInputLayout(Program* program) const;

private:
    mutable std::vector<D3D11_INPUT_ELEMENT_DESC> m_vD3DInputElementDescs;
    mutable std::tuple<std::vector<ID3D11Buffer*>, std::vector<UINT>, std::vector<UINT>> m_vD3DInputElementInfos;   // Buffer-stride-offset
    mutable std::vector<std::pair<Program*, ID3D11InputLayoutPtr>> m_vD3DInputLayouts;
};


SEEK_NAMESPACE_END
