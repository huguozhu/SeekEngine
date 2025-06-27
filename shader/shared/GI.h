#pragma once

// Global Illimination
#define VPL_NUM 64
struct GiRsmParam
{
    float4x4  light_view_proj_matrix;
    float4x4  inv_view_matrix;
    float2 radius_rsmsize;
    float2 t0;
};
struct GiLpvParam
{
    float4x4  inv_view_matrix;
	float lpv_attenuation;
	float lpv_power;
	float lpv_cutoff;	
	float t0;
};
struct VPL
{
    float3 pos;
    float3 normal;
    float3 flux;
    float  radius;
};

#define SH_COSINE_LOBE_C0 0.886226925f 	// sqrt(pi)/2
#define SH_COSINE_LOBE_C1 1.02332671f 	// sqrt(pi/3)
#define SH_C0 0.282094792f 				// 1 / 2sqrt(pi)
#define SH_C1 0.488602512f 				// sqrt(3/pi) / 2

#define LPV_DIM 32
#define LPV_DIM_HALF 16
#define LPV_DIM_INVERSE 0.03125f
#define LPV_SCALE 0.25f