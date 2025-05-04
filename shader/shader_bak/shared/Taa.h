#pragma once

struct TAAGlobalParams
{
    int4 statuses;
    float2 jitter;
    float2 invScreenSize;

#ifdef DVF_CPP
    TAAGlobalParams()
    {
        statuses = int4{ 0,0,0,0 };
    }
#endif
};
