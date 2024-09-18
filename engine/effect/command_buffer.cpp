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
    :Buffer()
{

}
CommandBuffer::CommandBuffer(size_t size)
    :Buffer(size), m_iPos(0)
{}
void CommandBuffer::Read(void* data, uint32_t size)
{
    SEEK_ASSERT(m_iPos + size <= m_iSize);
    memcpy_s(data, m_iSize, &m_pData[m_iPos], size);
    m_iPos += size;
}
void CommandBuffer::Write(void* data, uint32_t size)
{
    SEEK_ASSERT(m_iSize == 0);
    if (m_iPos + size > m_iBufSize)
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

/******************************************************************************
* RendererCommandManager
******************************************************************************/
RendererCommandManager::RendererCommandManager(Context* context)
    :m_pContext(context)
{
    m_pSubmitCommandBuffer = m_CommandBuffers[0];
    m_pRenderCommandBuffer = m_CommandBuffers[1];
}

CommandBuffer& RendererCommandManager::GetCommandBuffer(CommandType type)
{
    CommandBuffer& cb = type < CommandType::End ? m_pSubmitCommandBuffer[0] : m_pSubmitCommandBuffer[1];
    cb.Write((uint8_t)type);
    return cb;
}
void RendererCommandManager::ExecCommands(CommandBuffer& cb)
{
    cb.Reset();

    bool end = false;

    do
    {
        uint8_t command_type;
        cb.Read(command_type);
        switch (command_type)
        {
        case (uint8_t)CommandType::CreateVertexBuffer:
        {
            const Buffer* mem;
            VertexStreamInfo vsi;
            cb.Read(mem);
            cb.Read(vsi);
            break;
        }
        }
    } while (!end);

}
void RendererCommandManager::ExecPreCommands()
{
    this->ExecCommands(m_pSubmitCommandBuffer[0]);
}
void RendererCommandManager::ExecPostCommands()
{
    this->ExecCommands(m_pSubmitCommandBuffer[1]);
}
void RendererCommandManager::SwapCommandBuffer()
{
    CommandBuffer* tmp = m_pSubmitCommandBuffer;
    m_pSubmitCommandBuffer = m_pRenderCommandBuffer;
    m_pRenderCommandBuffer = tmp;
}
RenderBufferPtr RendererCommandManager::CreateVertexStream(Buffer* mem, VertexStreamInfo vsi, ResourceFlags flags)
{
    MutexScope ms(m_CommnadGenerateMutex);

    RenderBufferPtr buf = m_pContext->RHIContextInstance().CreateVertexBuffer(mem->Size(), flags, nullptr);
    CommandBuffer& cb = this->GetCommandBuffer(CommandType::CreateVertexBuffer);
    cb.Write(mem);
    cb.Write(vsi);
    return buf;
}




SEEK_NAMESPACE_END