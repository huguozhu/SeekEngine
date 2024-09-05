 #pragma once
#include "kernel/kernel.h"
#include "utils/buffer.h"
#include "thread/mutex.h"
#include "rhi/render_definition.h"

SEEK_NAMESPACE_BEGIN


enum class CommandType : uint8_t
{
    RendererInit,
    RendererShutdownBegin,
    CreateVertexBuffer,
    CreateIndexBuffer,
    CreateVertexLayout,
    CreateShader,
    CreateProgram,
    CreateTexture,
    CreateFrameBuffer,
    CreateShaderBuffer,
    End,
    RendererShutdownEnd,
    DestroyVertexBuffer,
    DestroyIndexBuffer,
    DestroyVertexLayout,
    DestroyShader,
    DestroyProgram,
    DestroyTexture,
    DestroyFrameBuffer,
    DestroyShaderBuffer,
};



class CommandBuffer : public Buffer
{
public:
    CommandBuffer(size_t size);
    void Read(void* data, uint32_t size);
    void Write(void* data, uint32_t size);
    void Align(uint32_t align);
        
    template<typename Type>
    void Read(Type& in)
    {
        this->Align(alignof(in));
        this->Read((void*)&in, sizeof(Type));
    }
    template<typename Type>
    void Write(const Type& in)
    {
        this->Align(alignof(Type));
        this->Write((void*)&in, sizeof(Type));
    }

protected:
    uint32_t    m_iPos;
};

struct VertexStreamLayout2
{
    uint32_t                buffer_offset = 0;
    VertexFormat            format = VertexFormat::Unknown;
    VertexElementUsage      usage = VertexElementUsage::Position;
    uint32_t                usage_index = 0;
    bool                    is_instance_attrib = false;
    uint32_t                instance_divisor = 1;
};

struct VertexStream2
{
    RenderBufferPtr                 render_buffer = nullptr;
    uint32_t                        offset = 0;
    uint32_t                        stride = 0;
    std::vector<VertexStreamLayout2> layouts;
    bool                            is_instance_stream = false;
};


class CommandGenerater
{
public:
    void CreateVertexLayout();
    BufferPtr CreateVertexBuffer(VertexStream2 vs, uint32_t flags);

protected:
    CommandBuffer& GetCommandBuffer(CommandType type);

private:
    Mutex   m_CommnadGenerateMutex;
    CommandBuffer m_CommandBufferPre;
    CommandBuffer m_CommandBufferPost;
    
};

SEEK_NAMESPACE_END