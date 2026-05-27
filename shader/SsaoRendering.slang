#include "Lighting.slangh"
#include "Common.slangh"
import Ssao;

static const int kernel_size = 64;
static const float radius = 0.5;
static const float bias = 0.025;

// VS cbuffer
cbuffer cb_SsaoVSParam
{
	SsaoParam vs_param;
}

// PS cbuffers / 资源
Texture2D gbuffer0;
Texture2D depth_tex;
Texture2D ssao_noise;
SamplerState noise_sampler;

cbuffer cb_CameraInfo
{
    CameraInfo cameraInfo;
}
cbuffer cb_SsaoPSParam
{
	SsaoParam ps_param;
}
cbuffer cb_SsaoSampleKernels
{
    float4 ssao_sample_kernels[kernel_size];
}

// VS 入口
void ssao_vs(
	in 	float3 iPos		: POSITION,
	out float4 oPosView	: POSITION,
	out float2 oTc		: TEXCOORD,
	out float4 oPos		: SV_POSITION)
{
	float4 pos = float4(iPos, 1.0);
	oPos = pos;
	oPosView = mul(pos, vs_param.inv_proj_matrix);
	oTc = TexCoordFromPosNDC(iPos);
}

// PS 入口
float4 ssao_ps(
	in 	float4 iPosView	: POSITION,
	in 	float2 iTc		: TEXCOORD)	: SV_Target
{
	float4 gb0 = gbuffer0.Sample(point_sampler, iTc);
	float3 normal_world = normalize(BestFitNormal_Decode(gb0.xyz));
	float4 normal_view = mul(float4(normal_world, 1.0), ps_param.view_matrix);
	float3 N = normalize(normal_view.xyz/normal_view.w);

	float depth = depth_tex.Sample(point_sampler, iTc).x;
	float depth_view = ViewSpaceDepth(depth, cameraInfo.nearPlane, cameraInfo.farPlane);
	float3 pos_view = (iPosView.xyz/iPosView.w) * (depth_view / cameraInfo.farPlane);

	// ssao_noise : [-1, 1], rotate around z-axis (in tangent space)
	float3 T = normalize(float3(ssao_noise.Sample(noise_sampler, iTc * ps_param.ssao_scale).xy, 0.0));

	// TBN : transform from tangent to view space
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);

	float occlusion = 0.0;
	for (int i=0; i<kernel_size; ++i)
	{
		// get sample position, ssao_sample_kernels = ([-1, 1], [-1, 1], [0, 1]), semisphere sample
		float3 sample_pos_view = mul(ssao_sample_kernels[i].xyz, TBN);
		sample_pos_view = pos_view + sample_pos_view * radius;

		// project sample position (to sample texture) (to get position on screen/texture)
		float4 pos_proj = mul(float4(sample_pos_view, 1.0), ps_param.proj_matrix);
		pos_proj.xyz /= pos_proj.w;
		float2 tc = TexCoordFromPosNDC(pos_proj.xyz);

		// get sample depth
		float sample_depth = depth_tex.Sample(point_sampler, tc).r;
		sample_depth = ViewSpaceDepth(sample_depth, cameraInfo.nearPlane, cameraInfo.farPlane);

		// range check & accumulate
		float range_check = smoothstep(0.0, 1.0, radius / abs(pos_view.z - sample_depth));
		occlusion += (sample_depth >= sample_pos_view.z + bias ? range_check : 0.0);
	}
	occlusion = 1.0 - (occlusion / (float)kernel_size);
	return occlusion;
}
