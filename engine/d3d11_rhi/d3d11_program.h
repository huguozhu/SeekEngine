#pragma once
#include "rendering/program.h"

DVF_NAMESPACE_BEGIN

class D3D11Program : public Program
{
public:
    D3D11Program(Context* context)
        : Program(context)
    { }
    ~D3D11Program() = default;

    DVFResult Active();
    void Deactive();
};

DVF_NAMESPACE_END
