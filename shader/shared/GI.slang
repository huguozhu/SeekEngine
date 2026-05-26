module GI;


// Global Illimination

public static const int VPL_NUM = 64;
public struct GiRsmParam
{
    public float4x4  light_view_proj_matrix;
    public float4x4  inv_view_matrix;
    public float2 radius_rsmsize;
    public float2 t0;
};
public struct GiLpvParam
{
    public float4x4  inv_view_matrix;
	public float lpv_attenuation;
	public float lpv_power;
	public float lpv_cutoff;	
	public float t0;
};
public struct VPL
{
    public float3 pos;
    public float3 normal;
    public float3 flux;
    public float  radius;
};

public static const float SH_COSINE_LOBE_C0 = 0.886226925f; // sqrt(pi)/2
public static const float SH_COSINE_LOBE_C1 = 1.02332671f; // sqrt(pi/3)
public static const float SH_C0 = 0.282094792f; // 1 / 2sqrt(pi)
public static const float SH_C1 = 0.488602512f; // sqrt(3/pi) / 2

public static const int LPV_SIZE = 32;
public static const int LPV_SIZE_HALF = 16;
public static const float LPV_SIZE_INVERSE = 0.03125f;
public static const float LPV_SCALE = 0.25f;
