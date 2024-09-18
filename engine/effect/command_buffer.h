 #pragma once
#include "kernel/kernel.h"
#include "kernel/context.h"
#include "utils/buffer.h"
#include "thread/mutex.h"
#include "rhi/render_definition.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * CommandBuffer
 ******************************************************************************/
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
        //this->Align(alignof(in));
        this->Read((void*)&in, sizeof(Type));
    }
    template<typename Type>
    void Write(const Type& in)
    {
        //this->Align(alignof(Type));
        this->Write((void*)&in, sizeof(Type));
    }
    void Reset() { m_iPos = 0; }

protected:
    uint32_t    m_iPos;
};


/******************************************************************************
* RendererCommandManager
******************************************************************************/
class RendererCommandManager
{
public:
    RendererCommandManager(Context* context);

    CommandBuffer& GetCommandBuffer(CommandType type);
    void    ExecPreCommands();
    void    ExecPostCommands();
    void    SwapCommandBuffer();

    /* All Command */
    RenderBufferPtr CreateVertexStream(Buffer* mem, VertexStreamInfo vsi, ResourceFlags flags);

private:
    void ExecCommands(CommandBuffer& cb);

private:
    Context*    m_pContext = nullptr;
    Mutex       m_CommnadGenerateMutex;

    CommandBuffer   m_CommandBuffers[2][2];
    CommandBuffer*   m_pSubmitCommandBuffer;
    CommandBuffer*   m_pRenderCommandBuffer;
};

SEEK_NAMESPACE_END