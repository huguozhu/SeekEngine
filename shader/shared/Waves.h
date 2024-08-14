#pragma once

struct WavesGlobalParams
{
    float4x4 mvpMatrix;

    float   spatial_step;
    float   wave_constant0;
    float   wave_constant1;
    float   wave_constant2;
    uint2   disturb_index;
    float   disturb_magnitude;
    float   unused;
};