#include "01.Tutorial.h"

#define DVF_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


Tutorial::Tutorial()
{
    m_pContext = MakeSharedPtr<Context>();
}

Tutorial::~Tutorial()
{
    
}


int main()
{
    Tutorial app;
    return APP_RUN(&app);
}

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
