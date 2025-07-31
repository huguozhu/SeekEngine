#pragma once

#include "rhi/base/rhi_mesh.h"
#include "rhi/d3d12/d3d12_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3D12Mesh : public RHIMesh
{
public:
    D3D12Mesh(Context* context);

    SResult Active(RHIProgram* program);

private:
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_vInputElemDescs;
    std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vVertexBufferViews;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    size_t m_iPsoHashValue;
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE m_eIndexBufferCutValue;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_ePrimitiveTopology;

};

SEEK_NAMESPACE_END
