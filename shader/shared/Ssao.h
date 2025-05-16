#pragma once

// Global Illimination
#define VPL_NUM 32
struct SsaoParam
{
	float4x4 inv_proj_matrix;	// for SsaoVS
    float4x4 view_matrix;		// others for SsaoPS
    float4x4 proj_matrix;
    float2 ssao_scale;
    float2 t0;
};

