#pragma once

struct Sprite2DGlobalParams
{
    float4 color;
    int srgb_to_rgb;
    float alpha;
    int2 valid_rt_size; // valid render target size
    int present_mode;
    float t0;
    float2 t1;

#ifdef DVF_CPP
    Sprite2DGlobalParams()
    {
        alpha = 1.0f;
        srgb_to_rgb = false;
        present_mode = false;
    }
#endif
};
