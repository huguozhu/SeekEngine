#include "01.Tutorial.h"

#define DVF_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


Tutorial::Tutorial()
{
    m_pContext = MakeSharedPtr<Context>();
}

Tutorial::~Tutorial()
{
    
}

SResult Tutorial::Run()
{
    RenderInitInfo info;
    SEEK_RETIF_FAIL(m_pContext->Init(info));

    return S_Success;
}


int main()
{
    Tutorial app;
    return app.Run();
}

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
