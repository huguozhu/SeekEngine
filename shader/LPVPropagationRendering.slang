#include "Common.slangh"
import GI;

// LPV 传播：VS + PS 合并
// VS 输出全屏三角形的三个顶点，PS 对每个 LPV 格子收集邻居的 SH 贡献

// VS 常量缓冲区：全屏三角形顶点
cbuffer cb_InjectVertics
{
	float4 vertices[3];
}

// PS 常量缓冲区 / 资源
Texture3D redSH;
Texture3D greenSH;
Texture3D blueSH;

cbuffer cb_RTIndex
{
	uint index;
}

struct PSInput
{
	float4 screenPos : SV_POSITION;
	float3 normal    : NORMAL;
	float3 flux      : POSITION;
	uint layerID     : SV_RenderTargetArrayIndex;
};

struct PSOutput
{
	float4 redSH      : SV_Target0;
	float4 greenSH    : SV_Target1;
	float4 blueSH     : SV_Target2;
	float4 acc_redSH  : SV_Target3;
	float4 acc_greenSH: SV_Target4;
	float4 acc_blueSH : SV_Target5;
};

static const float3 cellDirections[6] =
{
	float3(0, 0, 1),
	float3(1, 0, 0),
	float3(0, 0,-1),
	float3(-1,0, 0),
	float3(0, 1, 0),
	float3(0,-1, 0)
};

float4 DirCosLobeToSH(float3 direction)
{
	return float4(SH_COSINE_LOBE_C0, -SH_COSINE_LOBE_C1 * direction.y, SH_COSINE_LOBE_C1 * direction.z, -SH_COSINE_LOBE_C1 * direction.x);
}

struct SHContribution
{
	float4 red, green, blue;
};

static const float2 cellsides[4] = { float2(1.0, 0.0), float2(0.0, 1.0), float2(-1.0, 0.0), float2(0.0, -1.0) };
static const float directFaceSubtendedSolidAngle = 0.4006696846f / PI;
static const float sideFaceSubtendedSolidAngle = 0.4234413544f / PI;

float4 DirToSH(float3 direction)
{
	return float4(SH_C0, -SH_C1 * direction.y, SH_C1 * direction.z, -SH_C1 * direction.x);
}

float3 getEvalSideDirection(int index, int3 orientation)
{
	const float smallComponent = 0.4472135; // 1 / sqrt(5)
	const float bigComponent = 0.894427; // 2 / sqrt(5)

	const int2 side = cellsides[index];
	float3 tmp = float3(side.x * smallComponent, side.y * smallComponent, bigComponent);
	return float3(orientation.x * tmp.x, orientation.y * tmp.y, orientation.z * tmp.z);
}

float3 getReprojSideDirection(int index, int3 orientation)
{
	const int2 side = cellsides[index];
	return float3(orientation.x * side.x, orientation.y * side.y, 0);
}

SHContribution GetSHGatheringContribution(int4 cellIndex)
{
	SHContribution result = (SHContribution)0;
	for (int neighbourCell = 0; neighbourCell < 6; neighbourCell++)
	{
		int4 neighbourPos = cellIndex - int4(cellDirections[neighbourCell], 0);

		SHContribution neighbourContribution = (SHContribution)0;
		neighbourContribution.red = redSH.Load(neighbourPos);
		neighbourContribution.green = greenSH.Load(neighbourPos);
		neighbourContribution.blue = blueSH.Load(neighbourPos);

		// 主方向贡献
		float4 directionCosLobeSH = DirCosLobeToSH(cellDirections[neighbourCell]);
		float4 directionSH = DirToSH(cellDirections[neighbourCell]);
		result.red += directFaceSubtendedSolidAngle * dot(neighbourContribution.red, directionSH) * directionCosLobeSH;
		result.green += directFaceSubtendedSolidAngle * dot(neighbourContribution.green, directionSH) * directionCosLobeSH;
		result.blue += directFaceSubtendedSolidAngle * dot(neighbourContribution.blue, directionSH) * directionCosLobeSH;

		// 侧面方向贡献
		for (int face = 0; face < 4; face++)
		{
			float3 evaluatedSideDir = getEvalSideDirection(face, cellDirections[face]);
			float3 reproSideDir = getReprojSideDirection(face, cellDirections[face]);

			float4 evalSideDirSH = DirToSH(evaluatedSideDir);
			float4 reproSideDirCosLobeSH = DirCosLobeToSH(reproSideDir);

			result.red += sideFaceSubtendedSolidAngle * dot(neighbourContribution.red, evalSideDirSH) * reproSideDirCosLobeSH;
			result.green += sideFaceSubtendedSolidAngle * dot(neighbourContribution.green, evalSideDirSH) * reproSideDirCosLobeSH;
			result.blue += sideFaceSubtendedSolidAngle * dot(neighbourContribution.blue, evalSideDirSH) * reproSideDirCosLobeSH;
		}
	}

	return result;
}

// VS 入口：输出全屏三角形的顶点
float4 lpvpropagation_vs(uint vertex_id : SV_VertexID) : SV_Position
{
	float4 pos = vertices[vertex_id];
	return pos;
}

// PS 入口：收集邻居 SH 贡献并传播
PSOutput lpvpropagation_ps(float4 pos : SV_POSITION)
{
	int4 cellIndex = int4(pos.xy - 0.5, index, 0);
	SHContribution resultContribution = GetSHGatheringContribution(cellIndex);

	PSOutput output      = (PSOutput)0;
	output.redSH         = resultContribution.red;
	output.greenSH       = resultContribution.green;
	output.blueSH        = resultContribution.blue;
	output.acc_redSH     = resultContribution.red;
	output.acc_greenSH   = resultContribution.green;
	output.acc_blueSH    = resultContribution.blue;

	return output;
}
