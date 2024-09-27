#pragma once
#include "rhi/base/program.h"

SEEK_NAMESPACE_BEGIN

class D3D11Program : public Program
{
public:
    D3D11Program(Context* context)
        : Program(context)
    { }
    ~D3D11Program() = default;

    SResult Active();
    void Deactive();
};

SEEK_NAMESPACE_END
