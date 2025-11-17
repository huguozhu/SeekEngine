//STAGE=vs
cbuffer cb_Sprite2DInfo
{
    float4x4 mvp;
};

struct VSInput
{
    float3 posLocal     : POSITION;
    float2 texCoord     : TEXCOORD;
};

struct VSOutput
{
    float2 texCoord     : TEXCOORD;
    float4 pos          : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.texCoord = input.texCoord;
    output.pos 		= mul(float4(input.posLocal, 1.0), mvp);
    return output;
}
