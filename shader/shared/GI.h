#pragma once

// Global Illimination
#define VPL_NUM 32
struct GiRsmParam
{
    float4x4  inv_proj_matrix;              // for VS
    float4x4  light_view_proj_matrix;       // others for PS
    float4x4  inv_view_matrix;
    float2 radius_rsmsize;
    float2 t0;
};
struct VPL
{
    float3 pos;
    float3 normal;
    float3 flux;
    float  radius;
};
