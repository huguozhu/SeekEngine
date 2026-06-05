#pragma once

#include "kernel/kernel.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_render_state.h"
#include "utils/log.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>

SEEK_NAMESPACE_BEGIN

// Minimal Vulkan constant buffer helper - mirrors D3D11 CBufferAssignHelper
// but without D3D11 reflection dependency
class VkCBufferAssignHelper
{
public:
    VkCBufferAssignHelper(uint32_t bufferSize)
        : m_bufferSize(bufferSize)
    {
        m_data.resize(bufferSize, 0);
    }

    template <typename T>
    SResult AssignVariable(const T& var, uint32_t offset = 0)
    {
        if (offset + sizeof(T) > m_data.size())
            return ERR_INVALID_ARG;
        memcpy(m_data.data() + offset, &var, sizeof(T));
        return S_Success;
    }

    template <typename T>
    SResult AssignVariableByName(const char* /*name*/, const T& var, uint32_t offset = 0)
    {
        return AssignVariable(var, offset);
    }

    template <typename T>
    SResult AssignVariableByIndex(uint32_t /*idx*/, const T& var, uint32_t offset = 0)
    {
        return AssignVariable(var, offset);
    }

    void* Data() { return m_data.data(); }
    size_t Size() { return m_data.size(); }

private:
    uint32_t m_bufferSize;
    std::vector<uint8_t> m_data;
};

using VkCBufferAssignHelperPtr = std::shared_ptr<VkCBufferAssignHelper>;

SEEK_NAMESPACE_END

