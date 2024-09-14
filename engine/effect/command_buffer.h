 #pragma once
#include "kernel/kernel.h"
#include "kernel/context.h"
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
    CommandBuffer();
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

class Frame
{
public:
    Frame(Context* context);
    ~Frame();

    CommandBuffer& GetCommandBuffer(CommandType type);


private:
    Context*        m_pContext = nullptr;
    CommandBuffer   m_CommandBufferPre;
    CommandBuffer   m_CommandBufferPost;

};


class CommandGenerater
{
public:
    CommandGenerater(Context* context);

    void CreateVertexLayout();
    RenderBufferPtr CreateVertexStream(Buffer* mem, VertexStreamInfo vs, ResourceFlags flags);



private:
    Context*    m_pContext = nullptr;
    Mutex       m_CommnadGenerateMutex;
    
    
};

SEEK_NAMESPACE_END