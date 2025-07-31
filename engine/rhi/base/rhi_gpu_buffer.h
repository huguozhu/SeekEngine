#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_definition.h"
#include "math/quad_mesh_process.h"

SEEK_NAMESPACE_BEGIN

class RHIGpuBuffer
{
public:
    uint32_t            GetSize()           const { return m_iSize; }
    uint32_t            GetStructureStride()const { return m_iStructureStride; }
    ResourceFlags       GetResourceFlags()  const { return m_iFlags; }

    virtual SResult   Create(RHIGpuBufferData* buffer_data) = 0;    
    virtual SResult   Update(RHIGpuBufferData* buffer_data) = 0;
    virtual SResult   CopyBack(BufferPtr buffer, int start=0, int length=-1) = 0;

    SResult Update(const void* data, uint32_t size)
    {
        RHIGpuBufferData rbd{ size, data };
        return Update(&rbd);
    }

    SResult Create(const void* data, uint32_t size)
    {
        RHIGpuBufferData rbd{ size, data };
        return Create(&rbd);
    }

protected:
    RHIGpuBuffer(Context* context, uint32_t size, ResourceFlags flags,  uint32_t structure_stride = 0)
        : m_pContext(context), m_iSize(size), m_iFlags(flags), m_iStructureStride(structure_stride)
    {}
    virtual ~RHIGpuBuffer() {}

    Context*            m_pContext = nullptr;
    uint32_t            m_iSize = 0;
    uint32_t            m_iStructureStride = 0;
    ResourceFlags       m_iFlags = RESOURCE_FLAG_NONE;
};

SEEK_NAMESPACE_END
