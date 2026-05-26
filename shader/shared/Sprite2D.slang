module Sprite2D;


public struct Sprite2DGlobalParams
{
    public float4 color;
    public int srgb_to_rgb;
    public float alpha;
    public int2 valid_rt_size; // valid render target size
    public int present_mode;
    public float t0;
    public float2 t1;

#ifdef SEEK_CPP
    Sprite2DGlobalParams()
    {
        alpha = 1.0f;
        srgb_to_rgb = false;
        present_mode = false;
    }
#endif
};
