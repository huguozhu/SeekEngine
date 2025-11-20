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

#define MAX_METABALL_NUM 6
struct MetaballParam
{
	float width;
	float height;
	
    float surfaceThreshold;	
	int metaballCount;
	
	Metaball balls[MAX_METABALL_NUM];
};