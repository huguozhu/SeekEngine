#pragma once

#include "rhi/base/rhi_program.h"

SEEK_NAMESPACE_BEGIN

class D3D12Program : public RHIProgram
{
public:
    D3D12Program(Context* context)
        : RHIProgram(context)
    {
    }
    ~D3D12Program() = default;

    SResult Active();
    void Deactive();
};

SEEK_NAMESPACE_END
