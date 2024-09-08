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


class CommandGenerater
{
public:
    void CreateVertexLayout();
    BufferPtr CreateVertexStream(BufferPtr mem, VertexStreamInfo vs, uint32_t flags);

protected:
    CommandBuffer& GetCommandBuffer(CommandType type);

private:
    Mutex   m_CommnadGenerateMutex;
    CommandBuffer m_CommandBufferPre;
    CommandBuffer m_CommandBufferPost;
    
};

SEEK_NAMESPACE_END