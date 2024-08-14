//STAGE:ps
TextureCube skybox_tex;
SamplerState skybox_sampler;

float4 main(
	in float3 iTc : TEXCOORD) : SV_Target
{
	return skybox_tex.Sample(skybox_sampler, iTc);
}