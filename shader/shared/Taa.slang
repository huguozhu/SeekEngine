module Taa;


public struct TAAGlobalParams
{
    public int4 statuses;
    public float2 jitter;
    public float2 invScreenSize;

#ifdef DVF_CPP
    TAAGlobalParams()
    {
        statuses = int4{ 0,0,0,0 };
    }
#endif
};
