#include "01.Tutorial.h"

#define DVF_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


Tutorial::Tutorial()
    :AppFramework("Tutorial")
{
}

Tutorial::~Tutorial()
{
    
}

SResult Tutorial::OnCreate()
{

    return S_Success;
}
SResult Tutorial::OnUpdate()
{
    SEEK_RETIF_FAIL(m_pContext->Update());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->EndRender());

    return S_Success;
}


int main()
{
    Tutorial app;
    return APP_RUN(&app);
}

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
