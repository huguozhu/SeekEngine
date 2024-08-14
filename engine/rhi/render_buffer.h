#pragma once

#include "kernel/kernel.h"
#include "rhi/render_definition.h"
#include "math/quad_mesh_process.h"

SEEK_NAMESPACE_BEGIN

class RenderBuffer
{
public:
    uint32_t            GetSize()          const { return m_iSize; }
    ResourceFlags       GetResourceFlags() const { return m_iFlags; }

    virtual SResult   Create(RenderBufferData* buffer_data) = 0;
    virtual SResult   Update(RenderBufferData* buffer_data) = 0;
    virtual SResult   CopyBack(BufferPtr buffer, int start=0, int length=-1) = 0;

    SResult Update(const void* data, uint32_t size)
    {
        RenderBufferData rbd{ size, data };
        return Update(&rbd);
    }

    SResult Create(const void* data, uint32_t size)
    {
        RenderBufferData rbd{ size, data };
        return Create(&rbd);
    }

protected:
    RenderBuffer(Context* context, uint32_t size, ResourceFlags flags)
        : m_pContext(context), m_iSize(size), m_iFlags(flags)
    {}
    virtual ~RenderBuffer() {}

    Context*            m_pContext = nullptr;
    uint32_t            m_iSize = 0;
    ResourceFlags       m_iFlags = RESOURCE_FLAG_NONE;
};

SEEK_NAMESPACE_END
