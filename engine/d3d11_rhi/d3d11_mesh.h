/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_mesh.h
**
**      Brief                  : implement d3d11 mesh
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once
#include <vector>
#include <tuple>
#include "rendering/mesh.h"
#include "rendering_d3d11/d3d11_predeclare.h"

DVF_NAMESPACE_BEGIN

class D3D11Mesh : public Mesh
{
public:
    D3D11Mesh(Context* context);
    virtual ~D3D11Mesh() override;

    DVFResult Active(Program* program);
    DVFResult Deactive() const;
    ID3D11InputLayout* GetInputLayout(Program* program) const;

private:
    mutable std::vector<D3D11_INPUT_ELEMENT_DESC> m_vD3DInputElementDescs;
    mutable std::tuple<std::vector<ID3D11Buffer*>, std::vector<UINT>, std::vector<UINT>> m_vD3DInputElementInfos;   // Buffer-stride-offset
    mutable std::vector<std::pair<Program*, ID3D11InputLayoutPtr>> m_vD3DInputLayouts;
};


DVF_NAMESPACE_END
