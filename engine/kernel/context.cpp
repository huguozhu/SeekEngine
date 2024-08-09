#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"

SEEK_NAMESPACE_BEGIN

Context::Context()
{

}
Context::~Context()
{

}

SResult Context::Init(const RenderInitInfo& init_info)
{
    if (!m_pThreadManager)
    {
        m_pThreadManager = MakeUniquePtrMacro(ThreadManager, this);
        m_pThreadManager->Init();
    }
    return S_Success;
}
void Context::Uninit()
{
    m_pThreadManager.reset();
}

SResult Context::Update()
{
    return S_Success;
}
SResult Context::Render()
{
    return S_Success;
}
SResult Context::BeginFrame()
{
    return S_Success;
}
SResult Context::EndFrame()
{
    return S_Success;
}

SEEK_NAMESPACE_END