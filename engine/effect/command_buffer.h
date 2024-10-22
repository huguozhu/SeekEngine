 #pragma once
#include "kernel/kernel.h"
#include "kernel/context.h"
#include "utils/buffer.h"
#include "thread/mutex.h"
#include "rhi/base/rhi_definition.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * CommandBuffer
 ******************************************************************************/
enum class CommandType : uint8_t
{
    InitRenderer            = 0x01,
    InitEffect,
    RendererShutdownBegin,
    CreateMesh,
    CreateVertexBuffer,
    CreateIndexBuffer,

    CreateShader,
    CreateProgram,
    CreateTexture,
    CreateFrameBuffer,
    CreateShaderBuffer,

    End                     = 0x80,
    RendererShutdownEnd     = 0x81,
    DestroyMesh,
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
    void Finish();

protected:
    uint32_t    m_iPos = 0;
    uint32_t    m_iCommandSize = 0;
};


/******************************************************************************
* RendererCommandManager
******************************************************************************/
class RendererCommandManager
{
public:
    RendererCommandManager(Context* context);

    CommandBuffer& GetSubmitCommandBuffer(CommandType type);
    void    ExecPreRenderCommands();
    void    ExecPostRenderCommands();
    void    FinishSubmitCommandBuffer();
    void    SwapCommandBuffer();



public:
    /* All Renderer Commands */
    void InitRenderer(void* native_wnd);
    void InitEffect(Effect* effect);
     
    RHIMeshPtr CreateMesh();
    RHIRenderBufferPtr CreateVertexrBuffer(const void* data, uint32_t data_size, ResourceFlags flags);
    RHIRenderBufferPtr CreateIndexBuffer(const void* data, uint32_t data_size, ResourceFlags flags);
    


private:
    void ExecCommands(CommandBuffer& cb);

private:
    Context*    m_pContext = nullptr;
    Mutex       m_CommnadGenerateMutex;

    CommandBuffer   m_CommandBuffers0[2];
    CommandBuffer   m_CommandBuffers1[2];
    mutable CommandBuffer*  m_pSubmitCommandBuffer = nullptr;
    mutable CommandBuffer*  m_pRenderCommandBuffer = nullptr;
    
};

SEEK_NAMESPACE_END