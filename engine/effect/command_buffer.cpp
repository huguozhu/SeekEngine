#include "effect/command_buffer.h"
#include "effect/effect.h"

#include "thread/mutex.h"

#include "resource/resource_mgr.h"

#include "utils/log.h"
#include "utils/timer.h"
#include "math/hash.h"

#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_render_buffer.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * CommandBuffer
 ******************************************************************************/
CommandBuffer::CommandBuffer()
    :Buffer(), m_iPos(0)
{
    this->Finish();
}
CommandBuffer::CommandBuffer(size_t size)
    :Buffer(size), m_iPos(0)
{
    this->Finish();
}
void CommandBuffer::Read(void* data, uint32_t size)
{
    SEEK_ASSERT(m_iPos + size <= m_iCommandSize);
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
void CommandBuffer::Finish()
{
    CommandType cmd_type = CommandType::End;
    this->Write((uint8_t)cmd_type);
    m_iCommandSize = m_iPos;
    m_iPos = 0;
}
/******************************************************************************
 * RendererCommandManager
 ******************************************************************************/
RendererCommandManager::RendererCommandManager(Context* context)
    :m_pContext(context)
{
    m_pSubmitCommandBuffer = m_CommandBuffers0;
    m_pRenderCommandBuffer = m_CommandBuffers1;
}

CommandBuffer& RendererCommandManager::GetSubmitCommandBuffer(CommandType type)
{
    CommandBuffer& cb = type < CommandType::End ? m_pSubmitCommandBuffer[0] : m_pSubmitCommandBuffer[1];
    cb.Write((uint8_t)type);
    return cb;
}
void RendererCommandManager::ExecCommands(CommandBuffer& cb)
{
    cb.Reset();

    bool end = false;
    RHIContext& rc = m_pContext->RHIContextInstance();

    do
    {
        uint8_t command_type;
        cb.Read(command_type);
        switch (command_type)
        {
        case (uint8_t)CommandType::InitRenderer:
        {
            rc.Init();
            void* native_wnd = nullptr;
            cb.Read(native_wnd);
            if (native_wnd)
            {
                rc.AttachNativeWindow("", native_wnd);
                rc.SetFinalRHIFrameBuffer(rc.GetScreenRHIFrameBuffer());
            }
            break;
        }
        case (uint8_t)CommandType::InitEffect:
        {
            Effect* pEffect = nullptr;
            cb.Read(pEffect);
            pEffect->Initialize();
            break;
        }
        case (uint8_t)CommandType::CreateMesh:
        {
            RHIMesh* pRHIMesh = nullptr;
            cb.Read(pRHIMesh);
            pRHIMesh->Init();
            break;
        }
        case (uint8_t)CommandType::CreateVertexBuffer:
        case (uint8_t)CommandType::CreateIndexBuffer:
        {
            RHIRenderBuffer* buf = nullptr;
            void* data = nullptr;
            uint32_t data_size = 0;
            cb.Read(buf);
            cb.Read(data);
            cb.Read(data_size);
            buf->Create(data, data_size);
            break;
        }
        case (uint8_t)CommandType::CreateTextureFromBitmap:
        {
            RHITexture* pTex = nullptr;
            BitmapBuffer* pBitmap = nullptr;
            cb.Read(pTex);
            cb.Read(pBitmap);
            pTex->Create(std::vector<BitmapBufferPtr>{pBitmap->shared_from_this()});
            break;
        }
        case (uint8_t)CommandType::End:
        {
            end = true;
            break;
        }
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
    m_pSubmitCommandBuffer[0].Finish();
    m_pSubmitCommandBuffer[1].Finish();
}
void RendererCommandManager::SwapCommandBuffer()
{
    CommandBuffer* tmp = m_pSubmitCommandBuffer;
    m_pSubmitCommandBuffer = m_pRenderCommandBuffer;
    m_pRenderCommandBuffer = tmp;
}


/******************************************************************************
* All Renderer Commands, Run in Main Thread
******************************************************************************/
void RendererCommandManager::InitRenderer(void* native_wnd)
{
    if (!m_pContext->IsMultiThreaded())
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        rc.Init();        
        if (native_wnd)
        {
            rc.AttachNativeWindow("", native_wnd);
            rc.SetFinalRHIFrameBuffer(rc.GetScreenRHIFrameBuffer());
        }
        return;
    }

    MutexScope ms(m_CommnadGenerateMutex);
    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::InitRenderer);
    cb.Write(native_wnd);
    return;
}
void RendererCommandManager::InitEffect(Effect* effect)
{
    if (!m_pContext->IsMultiThreaded())
    {
        effect->Initialize();
        return;
    }
    MutexScope ms(m_CommnadGenerateMutex);
    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::InitEffect);
    cb.Write(effect);
    return;
}
RHIMeshPtr RendererCommandManager::CreateMesh()
{
    if (!m_pContext->IsMultiThreaded())
        return m_pContext->RHIContextInstance().CreateMesh();

    MutexScope ms(m_CommnadGenerateMutex);
    RHIMeshPtr pMesh = m_pContext->RHIContextInstance().CreateMesh();

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateMesh);
    cb.Write(pMesh.get());
    return pMesh;
}
RHIRenderBufferPtr RendererCommandManager::CreateVertexrBuffer(const void* data, uint32_t data_size, ResourceFlags flags)
{
    if (!m_pContext->IsMultiThreaded())
        return m_pContext->RHIContextInstance().CreateVertexBuffer(data_size, flags, new RHIRenderBufferData(data_size, data));

    MutexScope ms(m_CommnadGenerateMutex);
    RHIRenderBufferPtr buf = m_pContext->RHIContextInstance().CreateEmptyVertexBuffer(data_size, flags);

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateVertexBuffer);
    cb.Write(buf.get());
    cb.Write(data);
    cb.Write(data_size);
    return buf;
}

RHIRenderBufferPtr RendererCommandManager::CreateIndexBuffer(const void* data, uint32_t data_size, ResourceFlags flags)
{
    if (!m_pContext->IsMultiThreaded())
        return m_pContext->RHIContextInstance().CreateVertexBuffer(data_size, flags, new RHIRenderBufferData(data_size, data));

    MutexScope ms(m_CommnadGenerateMutex);
    RHIRenderBufferPtr buf = m_pContext->RHIContextInstance().CreateEmptyIndexBuffer(data_size, flags);

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateIndexBuffer);
    cb.Write(buf.get());
    cb.Write(data);
    cb.Write(data_size);
    return buf;
}

RHITexturePtr RendererCommandManager::CreateTexture2D(RHITexture::Desc desc, BitmapBuffer* pBitmap)
{
    if (!m_pContext->IsMultiThreaded())
        return m_pContext->RHIContextInstance().CreateTexture2D(desc, pBitmap->shared_from_this());

    MutexScope ms(m_CommnadGenerateMutex);
    RHITexturePtr tex = m_pContext->RHIContextInstance().CreateEmptyTexture2D(desc);

    CommandBuffer& cb = this->GetSubmitCommandBuffer(CommandType::CreateTextureFromBitmap);
    cb.Write(tex.get());
    cb.Write(pBitmap);
    return tex;
}



SEEK_NAMESPACE_END