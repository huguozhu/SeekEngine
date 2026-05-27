import SkyBox;

// SkyBox VS 和 PS 合并到一个文件
// VS 和 PS 共用 cbuffer + texture/sampler 声明

cbuffer cb_SkyBoxParams
{
    SkyBoxParams params;
}

TextureCube skybox_tex;
SamplerState skybox_sampler;

// VS 入口
void skybox_vs(
	in float3 iPos	: POSITION,
	out float3 oTc	: TEXCOORD,
	out float4 oPos	: SV_POSITION
)
{
	float4 pos = float4(iPos, 1.0);
	oTc = mul(pos, params.inv_mvp).xyz;
	oPos = pos;
}

// PS 入口
float4 skybox_ps(
	in float3 iTc : TEXCOORD) : SV_Target
{
	return skybox_tex.Sample(skybox_sampler, iTc);
}
