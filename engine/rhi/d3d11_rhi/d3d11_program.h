#pragma once
#include "rhi/base/program.h"

SEEK_NAMESPACE_BEGIN

class D3D11RHIProgram : public RHIProgram
{
public:
    D3D11RHIProgram(Context* context)
        : RHIProgram(context)
    { }
    ~D3D11RHIProgram() = default;

    SResult Active();
    void Deactive();
};

SEEK_NAMESPACE_END
