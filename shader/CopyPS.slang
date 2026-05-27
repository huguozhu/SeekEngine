//STAGE:ps
#include "Common.slangh"

Texture2D src_tex;

float4 main(float2 iTc : TEXCOORD) : SV_Target
{
	return src_tex.Sample(point_sampler, iTc);
}


