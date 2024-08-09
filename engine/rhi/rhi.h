#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN



class RHIContext
{
public:
    virtual SResult Init() = 0;
    /** Called after the RHI is initialized; before the render thread is started  */
    virtual SResult PostInit() {}

    virtual const std::string GetName() const = 0;

    virtual void* CreateSamplerState() = 0;
    virtual void* CreateRasterizerState() = 0;
    virtual void* CreateTexture() = 0;


protected:
    std::vector<IndexBufferPtr>     m_vIndexBuffers;
    std::vector<VertexBufferPtr>    m_vVertexBuffers;
    std::vector<FrameBufferPtr>     m_vFrameBuffers;

};


SEEK_NAMESPACE_END