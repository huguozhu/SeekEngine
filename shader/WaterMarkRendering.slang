#include "Common.slangh"
import WaterMark;

// 水印渲染：VS + PS 合并
// VS 将 NDC 坐标变换为屏幕坐标，PS 采样水印纹理

// VS/PS 共享的常量缓冲区
cbuffer waterMarkDesc
{
	WaterMarkDesc desc;
};

// PS 独有资源
cbuffer targetSize
{
	float target_width;
	float target_height;
};
Texture2D watermark_tex;

struct VSInput
{
	float3 posNDC : POSITION;
};

struct VSOutput
{
	float2 texCoord : TEXCOORD;
	float4 pos      : SV_POSITION;
};

// VS 入口：NDC → 屏幕坐标
VSOutput watermark_vs(VSInput input)
{
	VSOutput output;
	output.texCoord = TexCoordFromPosNDC(input.posNDC);
	float2 pos = input.posNDC.xy * 1.0;
	output.pos = float4(pos.xy, input.posNDC.z, 1.0);
	return output;
}

// PS 入口：采样水印纹理
float4 watermark_ps(in float2 iTc : TEXCOORD) : SV_Target
{
	float2 tc = iTc - float2(0.5, 0.5);

	float v = (float)target_width / (float)target_height;
	tc.x *= v;
	float radian = desc.radian;
	tc = mul(float2x2(
	cos(radian), -sin(radian),
	sin(radian), cos(radian)), tc);
	tc.x /= v;

	tc += float2(0.5, 0.5);
	float4 final = watermark_tex.Sample(linear_sampler, tc);
	return final;
}
