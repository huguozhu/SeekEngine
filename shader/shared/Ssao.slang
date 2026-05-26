module Ssao;


// Global Illimination
public static const int VPL_NUM = 32;
public struct SsaoParam
{
	public float4x4 inv_proj_matrix;	// for SsaoVS
    public float4x4 view_matrix;		// others for SsaoPS
    public float4x4 proj_matrix;
    public float2 ssao_scale;
    public float2 t0;
};

