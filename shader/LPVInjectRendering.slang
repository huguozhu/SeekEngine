#include "Lighting.slangh"
#include "Common.slangh"
import GI;

// LPV 注入：VS + PS 合并
// VS 将 RSM 纹素变换为 LPV 格子坐标，PS 计算 SH 系数注入 3D 纹理

Texture2D rsm_color0;
Texture2D rsm_color1;
Texture2D rsm_color2;

// VS/PS 共享的常量缓冲区
cbuffer cb_RTIndex
{
	uint index;
}

struct RSMTexel
{
	float3 rsm_normal;
	float3 rsm_position;
	float3 rsm_flux;
};

// 统一的 PSInput：layerID 由硬件光栅化填充，VS 不输出
struct PSInput
{
	float4 screenPos : SV_POSITION;
	float3 normal    : NORMAL;
	float3 flux      : POSITION;
	uint layerID     : SV_RenderTargetArrayIndex;
};

struct PSOutput
{
	float4 redSH   : SV_Target0;
	float4 greenSH : SV_Target1;
	float4 blueSH  : SV_Target2;
};

RSMTexel GetRSMTexel(uint2 coords)
{
	RSMTexel texel = (RSMTexel)0;

	float3 rsm0        = rsm_color0.Load(int3(coords, 0)).xyz;
	texel.rsm_normal   = normalize(BestFitNormal_Decode(rsm0));
	texel.rsm_position = rsm_color1.Load(int3(coords, 0)).xyz;
	texel.rsm_flux     = rsm_color2.Load(int3(coords, 0)).xyz;
	return texel;
}

float4 DirCosLobeToSH(float3 direction)
{
	return float4(SH_COSINE_LOBE_C0, -SH_COSINE_LOBE_C1 * direction.y, SH_COSINE_LOBE_C1 * direction.z, -SH_COSINE_LOBE_C1 * direction.x);
}

// VS 入口：将 RSM 纹素投影到 LPV 格子
PSInput lpvinject_vs(uint vertex_id : SV_VertexID)
{
	PSInput output = (PSInput)0;

	uint2 RSMsize;
	rsm_color0.GetDimensions(RSMsize.x, RSMsize.y);
	uint2 rsmCoords = uint2(vertex_id % RSMsize.x, vertex_id / RSMsize.x);

	RSMTexel texel = GetRSMTexel(rsmCoords);

	/* 将虚拟点光源沿法线移动半个格子，防止自光照与自阴影 */
	float3 pos_ws = texel.rsm_position;
	float4 cellPos = float4(int3(pos_ws * LPV_SCALE + float3(LPV_SIZE_HALF, LPV_SIZE_HALF, LPV_SIZE_HALF) + 0.5f * texel.rsm_normal), 1.0f);

	output.screenPos = float4((cellPos.xy + 0.5f) * LPV_SIZE_INVERSE * 2.0f - 1.0f, (cellPos.z + 0.5 - index) * LPV_SIZE_INVERSE, 1.0f);
	output.screenPos.y = -output.screenPos.y;

	output.normal = texel.rsm_normal;
	output.flux = texel.rsm_flux;
	return output;
}

// PS 入口：计算 SH 系数并注入
PSOutput lpvinject_ps(PSInput input)
{
	PSOutput output = (PSOutput)0;

	float4 SH_coef = DirCosLobeToSH(input.normal) / PI;
	output.redSH = SH_coef * input.flux.r;
	output.greenSH = SH_coef * input.flux.g;
	output.blueSH = SH_coef * input.flux.b;

	return output;
}
