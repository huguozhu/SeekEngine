#include "effect/command_buffer.h"

#include "thread/mutex.h"

#include "resource/resource_mgr.h"

#include "utils/log.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * CommandBuffer
 ******************************************************************************/
CommandBuffer::CommandBuffer()
    :Buffer(), m_iPos(0)
{

}
CommandBuffer::CommandBuffer(size_t size)
    :Buffer(size), m_iPos(0)
{}
void CommandBuffer::Read(void* data, uint32_t size)
{
    SEEK_ASSERT(m_iPos + size <= m_iSize);
    memcpy_s(data, size, &m_pData[m_iPos], size);
    m_iPos += size;
}
void CommandBuffer::Write(void* data, uint32_t size)
{
    if (m_iPos + size > m_iSize)
    {   
        this->Expand(m_iSize + (16 << 10));
    }
    memcpy_s(&m_pData[m_iPos], m_iSize, data, size);
    m_iPos += size;
}
void CommandBuffer::Align(uint32_t align)
{
    m_iPos = seek_alignaddr(m_iPos, align);
}
void CommandBuffer::Finish()
{
    CommandType cmd_type = CommandType::End;
    this->Write((uint8_t)cmd_type);
    m_iPos = 0;
}
/******************************************************************************
 * RendererCommandManager
 ******************************************************************************/
RendererCommandManager::RendererCommandManager(Context* context)
    :m_pContext(context)
{
    m_pSubmitCommandBuffer = m_CommandBuffers[0];
    m_pRenderCommandBuffer = m_CommandBuffers[1];
}

CommandBuffer& RendererCommandManager::GetSubmitCommandBuffer(CommandType type)
{
    CommandBuffer& cb = type < CommandType::End ? m_pSubmitCommandBuffer[0] : m_pSubmitCommandBuffer[1];
    cb.Write((uint8_t)type);
    return cb;
}
void RendererCommandManager::ExecCommands(CommandBuffer& cb)
{
    LOG_RECORD_FUNCTION();
    cb.Reset();

    bool end = false;

    do
    {
        uint8_t command_type;
        cb.Read(command_type);
        LOG_INFO("command_type = %d", (uint32_t)command_type)
        switch (command_type)
        {
        case (uint8_t)CommandType::RendererInit:
        {            
            m_pContext->RHIContextInstance().Init();
            void* native_wnd = nullptr;
            cb.Read(native_wnd);
            if (native_wnd)
            {
                RHIContext& rc = m_pContext->RHIContextInstance();
                rc.AttachNativeWindow("", native_wnd);
                rc.SetFinalFrameBuffer(rc.GetScreenFrameBuffer());
            }
            break;
        }
        case (uint8_t)CommandType::CreateVertexBuffer:
        {
            const Buffer* mem;
            VertexStreamInfo vsi;
            cb.Read(mem);
            cb.Read(vsi);
            break;
        }
        case (uint8_t)CommandType::End:
            end = true;
            break;

        }
    } while (!end);

}
void RendererCommandManager::ExecPreRenderCommands()
{
    this->ExecCommands(m_pRenderCommandBuffer[0]);
    m_pRenderCommandBuffer[0].Reset();
}
void RendererCommandManager::ExecPostRenderCommands()
{
    this->ExecCommands(m_pRenderCommandBuffer[1]);
    m_pRenderCommandBuffer[1].Reset();
}
void RendererCommandManager::FinishSubmitCommandBuffer()
{
    LOG_RECORD_FUNCTION();
    m_pSubmitCommandBuffer[0].Finish();
    m_pSubmitCommandBuffer[1].Finish();
    LOG_INFO("m_pSubmitCommandBuffer[0] = %x \n", &m_pSubmitCommandBuffer[0]);
    LOG_INFO("m_pSubmitCommandBuffer[1] = %x \n", &m_pSubmitCommandBuffer[1]);

}
void RendererCommandManager::SwapCommandBuffer()
{
    LOG_RECORD_FUNCTION();
    CommandBuffer* tmp = m_pSubmitCommandBuffer;
    m_pSubmitCommandBuffer = m_pRenderCommandBuffer;
    m_pRenderCommandBuffer = tmp;
}


/******************************************************************************
* All Renderer Commands, Run in Main Thread
******************************************************************************/
void RendererCommandManager::InitRendererInit(void* native_wnd)
{
    MutexScope ms(m_CommnadGenerateMutex);
    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::RendererInit);
    cb.Write(native_wnd);
    return;
}
VertexStreamHandle RendererCommandManager::CreateVertexStream(Buffer* mem, VertexStreamInfo vsi, ResourceFlags flags)
{
    MutexScope ms(m_CommnadGenerateMutex);

    VertexStreamHandle handle = this->AllocVertexStreamHandle();

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateVertexBuffer);
    cb.Write(handle);
    cb.Write(mem);
    cb.Write(vsi);
    cb.Write(flags);
    return handle;
}
MeshHandle RendererCommandManager::CreateMesh()
{
    MutexScope ms(m_CommnadGenerateMutex);

    MeshHandle handle = this->AllocMeshHandle();

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateMesh);
    cb.Write(handle);
    return handle;
}



SEEK_NAMESPACE_END