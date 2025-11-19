#pragma once

struct Metaball
{
    float3 position;
    float radius;
    float3 velocity;
    float intensity;
#ifdef SEEK_CPP
    Metaball()
        : position(0, 0, 0), velocity(0, 0, 0), radius(0.3f), intensity(1.0f)
    {
    }
#endif
};