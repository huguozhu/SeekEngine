#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN

/*
 * All RHI Ojbect's all initialize function using Graphic API run in Init()
 * RHIObject::Init() run in Rendering Thread.
 */
class RHIObject 
{
public:
    virtual SResult Init() { return S_Success; }

};

SEEK_NAMESPACE_END