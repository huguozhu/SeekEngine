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
    RendererInit,
    RendererShutdownBegin,
    CreateMesh,
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
    void Finish();

protected:
    uint32_t    m_iPos;
};


/******************************************************************************
* RendererCommandManager
******************************************************************************/
#define SEEK_INVALID_HANDLE  ((uint16_t)-1)
#define SEEK_HANDLE(_name)                   \
    struct _name { uint16_t index = -1; };        \
    inline bool IsValid(_name _handle) { return SEEK_INVALID_HANDLE != _handle.index; }

#define SEEK_RESOURCE_FUNCTIONS(name)                       \
    private: std::vector<name>           m_v##name##s;      \
    public: name&       Alloc##name() {                     \
        m_v##name##s.emplace_back(name());                  \
        name& v = m_v##name##s.back();                      \
        v.index = m_v##name##s.size() -1;                   \
        return v;                                           \
    }

SEEK_HANDLE(VertexStreamHandle);
SEEK_HANDLE(MeshHandle);
class RendererCommandManager
{
public:
    RendererCommandManager(Context* context);

    CommandBuffer& GetSubmitCommandBuffer(CommandType type);
    void    ExecPreRenderCommands();
    void    ExecPostRenderCommands();
    void    FinishSubmitCommandBuffer();
    void    SwapCommandBuffer();

    SEEK_RESOURCE_FUNCTIONS(VertexStreamHandle);
    SEEK_RESOURCE_FUNCTIONS(MeshHandle);


public:
    /* All Renderer Commands */
    void InitRendererInit(void* native_wnd);
    VertexStreamHandle CreateVertexStream(Buffer* mem, VertexStreamInfo vsi, ResourceFlags flags);
    MeshHandle CreateMesh();


private:
    void ExecCommands(CommandBuffer& cb);

private:
    Context*    m_pContext = nullptr;
    Mutex       m_CommnadGenerateMutex;

    CommandBuffer   m_CommandBuffers[2][2];
    CommandBuffer*  m_pSubmitCommandBuffer;
    CommandBuffer*  m_pRenderCommandBuffer;



};

SEEK_NAMESPACE_END