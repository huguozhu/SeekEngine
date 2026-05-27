//PREDEFINE:HAS_TEX=0,1
import ParticleCommonTypes;
#include "Common.slangh"

// 粒子渲染：VS + PS 合并
// HAS_TEX 预定义控制是否使用纹理采样

// VS 常量缓冲区 / 资源
cbuffer CParticleVertices
{
	float4 particle_vertices_tc[6];
};

cbuffer CParticleRenderParam
{
	GpuRenderParam param;
};
StructuredBuffer<Particle> particle_datas;
StructuredBuffer<SortInfo>	sort_indices;

Texture2D color_over_life_tex;
Texture2D size_over_life_tex;

// PS 纹理（仅 HAS_TEX=1 时需要）
#if HAS_TEX == 1
Texture2D particle_tex;
#endif

// VS 入口：将粒子顶点从世界空间变换到屏幕空间
void particle_vs(
	uint instance_id 	: SV_InstanceID,
	uint vertex_id 		: SV_VertexID,
#if HAS_TEX == 1
	out float2 oTc		: TEXCOORD,
#endif
	out float4 oColor	: COLOR,
	out float4 oPos		: SV_POSITION
)
{
	Particle particle = particle_datas[sort_indices[instance_id].particle_index];

	float life = particle.life_time.x / particle.life_time.y;
	float2 tc_color = float2(life, 0.5);
	oColor 		= color_over_life_tex.SampleLevel(linear_sampler, tc_color, 0.0);
	float2 size = size_over_life_tex.SampleLevel(linear_sampler, tc_color, 0.0).rg;

	float4 quad_pos = float4(particle_vertices_tc[vertex_id].xy, 0.0, 1.0);
	quad_pos.xy *= size;

	float4 pos = mul(float4(particle.position.xyz, 1.0), param.view_matrix);
	pos.xyz += quad_pos.xyz;
	pos = mul(pos, param.proj_matrix);
	oPos = pos;

#if HAS_TEX == 1
	uint row_index = uint(particle.tex_index / param.tex_rows_cols.x);
	uint col_index = uint(particle.tex_index % param.tex_rows_cols.x);
	float2 tc_in = particle_vertices_tc[vertex_id].zw;
	float tc_x_clip = 1.0 / float(param.tex_rows_cols.y);
	float tc_y_clip = 1.0 / float(param.tex_rows_cols.x);

	float tc_x = tc_x_clip * (col_index + uint(tc_in.x));
	float tc_y = 1.0 - tc_y_clip * (row_index + uint(tc_in.y));
	oTc = float2(tc_x, tc_y);
#endif
}

// PS 入口：采样粒子纹理或直接输出颜色
float4 particle_ps(
#if HAS_TEX == 1
	in float2 iTc 		: TEXCOORD,
#endif
	in float4 iColor	: COLOR) : SV_Target
{
#if HAS_TEX == 1
	float4 sColor = particle_tex.Sample(linear_sampler, iTc);
	return iColor * sColor;
#endif
#if HAS_TEX == 0
	return iColor;
#endif
}
