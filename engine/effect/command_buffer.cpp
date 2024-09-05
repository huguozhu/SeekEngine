#include "effect/command_buffer.h"

#include "thread/mutex.h"

#include "utils/log.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * CommandBuffer
 ******************************************************************************/
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
* CommandGenerater
******************************************************************************/
void CommandGenerater::CreateVertexLayout()
{

}
BufferPtr CommandGenerater::CreateVertexBuffer(VertexStream2 vs, uint32_t flags)
{
    MutexScope ms(m_CommnadGenerateMutex);

    
    CommandBuffer& cmd = this->GetCommandBuffer(CommandType::CreateVertexBuffer);

    return nullptr;
}


CommandBuffer& CommandGenerater::GetCommandBuffer(CommandType type)
{
    CommandBuffer& cb = type < CommandType::End ? m_CommandBufferPre : m_CommandBufferPost;
    cb.Write((uint8_t)type);
    return cb;
}


SEEK_NAMESPACE_END