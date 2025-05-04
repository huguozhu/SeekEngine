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

struct Sprite2DAdvanceGlobalParams
{
    int4 rounded_corner_enable;
    int2 rect_center;
    int2 rect_corner;
    int rounded_corner_radius;
    int border_width;
    int2 t0;
    float4 border_simple_color;

#ifdef DVF_CPP
    Sprite2DAdvanceGlobalParams()
    {
        rounded_corner_enable = int4{-1, -1, -1, -1};
        rounded_corner_radius = 0;
        border_width = 20;
        border_simple_color = float4(1,1,1,1);
    }
#endif
};
