//STAGE:vs
import SkyBox;

cbuffer cb_SkyBoxParams
{
    SkyBoxParams params;
}

void main(
	in float3 iPos	: POSITION,
	out float3 oTc	: TEXCOORD,
	out float4 oPos	: SV_POSITION
)
{
	float4 pos = float4(iPos, 1.0);
	oTc = mul(pos, params.inv_mvp).xyz;
	oPos = pos;
}