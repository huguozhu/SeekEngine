module Waves;


public struct WavesGlobalParams
{
    public float4x4 mvpMatrix;

    public float   spatial_step;
    public float   wave_constant0;
    public float   wave_constant1;
    public float   wave_constant2;
    public uint2   disturb_index;
    public float   disturb_magnitude;
    public float   unused;
};
