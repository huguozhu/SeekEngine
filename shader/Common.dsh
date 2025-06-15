#pragma once

#include "shared/common.h"

static const float PI 					= 3.1415926;
static const float3 RGB_TO_LUM			= float3(0.2126, 0.7152, 0.0722);

SamplerState linear_sampler;
SamplerState point_sampler;
SamplerState shadow_map_sampler;

float4x4 MatrixInverse(float4x4 mat)
{
    float n11 = mat[0][0], n12 = mat[1][0], n13 = mat[2][0], n14 = mat[3][0];
    float n21 = mat[0][1], n22 = mat[1][1], n23 = mat[2][1], n24 = mat[3][1];
    float n31 = mat[0][2], n32 = mat[1][2], n33 = mat[2][2], n34 = mat[3][2];
    float n41 = mat[0][3], n42 = mat[1][3], n43 = mat[2][3], n44 = mat[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float r00 = t11 * idet;
    float r01 = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    float r02 = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    float r03 = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    float r10 = t12 * idet;
    float r11 = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    float r12 = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    float r13 = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    float r20 = t13 * idet;
    float r21 = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    float r22 = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    float r23 = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    float r30 = t14 * idet;
    float r31 = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    float r32 = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    float r33 = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;
	
	float4x4 ret = float4x4(r00, r01, r02, r03, r10, r11, r12, r13, r20, r21, r22, r23, r30, r31, r32, r33);

    return ret;
}

float2 TexCoordFromPosNDC(float3 posNDC)
{
	float2 texCoord = posNDC.xy * 0.5f;
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
	texCoord.y *= 1;
#else
    texCoord.y *= -1;
#endif	
	texCoord += 0.5f;
	return texCoord;
}

float2 TexCoordToPosNDC(float2 texCoord)
{
	float2 posNDC = texCoord - 0.5f;
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
	posNDC.y *= 1;
#else
    posNDC.y *= -1;
#endif	
	posNDC *= 2.0f;
	return posNDC;
}

// z : depth buffer Z-value
// n : near_plane
// f : far_plane
// "depth buffer Z-value" --> "eye Z value"
float ViewSpaceDepth(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return (2.0 * n * f) / (f + n - z * (f - n));
#else
	return (n * f) / (f - z	* (f - n));
#endif
}

// "depth buffer Z-value" --> "linear [0..1] range (near plane to far plane)"
float LinearizeDepth(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return n * (z + 1.0) / (f + n - z * (f - n));
#else
	return z / (f - z * (f - n));
#endif
}

// "depth buffer Z-value" --> "linear [0..1] range (eye to far plane)"
float LinearizeDepth2(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return (2 * n) / (f + n - z * (f - n));
#else
	return n / (f - z * (f - n));
#endif
}

float LinearDetpthToNonLinear(float depth, float cam_near, float cam_far)
{
	float q = cam_far / (cam_far - cam_near);
	return (depth * q - cam_near * q) / depth;
}

static const float3x3 XYZ_2_LinearRGB_MAT =
{
	 3.2409699419, -1.5373831776, -0.4986107603,
	-0.9692436363,  1.8759675015,  0.0415550574,
	 0.0556300797, -0.2039769589,  1.0569715142,
};

static const float3x3 LinearRGB_2_XYZ_MAT =
{
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041,
};

float3 LinearRGB_2_XYZ( float3 LinearRGB )
{
	return mul(LinearRGB_2_XYZ_MAT, LinearRGB);
}

float3 XYZ_2_LinearRGB( float3 XYZ )
{
	return mul(XYZ_2_LinearRGB_MAT, XYZ);
}

static const float3 XYZ_WHITE_REF_D65 = float3(0.95047, 1.000, 1.08883);
static const float3 XYZ_WHITE_REF_D50 = float3(0.966797, 1.000, 0.825188);
static const float XYZ_2_LAB_DELTA_SQUARED = 0.04280618311; // (6/29)^3
static const float XYZ_2_LAB_DELTA_CUBED = 0.00885645167; // (6/29)^3

float xyz_otherwise(float t)
{
	return (t / (3.0 * XYZ_2_LAB_DELTA_SQUARED)) + 4.0 / 29.0;
}

float3 LinearRGB_2_LAB( float3 LinearRGB, float3 ReferenceWhite )
{
	float3 XYZ = LinearRGB_2_XYZ(LinearRGB);

	float t_X = XYZ.x / ReferenceWhite.x;
	float t_Y = XYZ.y / ReferenceWhite.y;
	float t_Z = XYZ.z / ReferenceWhite.z;

	float f_X = (t_X > XYZ_2_LAB_DELTA_CUBED) ? pow(t_X, 1.0 / 3.0) : xyz_otherwise(t_X);
	float f_Y = (t_Y > XYZ_2_LAB_DELTA_CUBED) ? pow(t_Y, 1.0 / 3.0) : xyz_otherwise(t_Y);
	float f_Z = (t_Z > XYZ_2_LAB_DELTA_CUBED) ? pow(t_Z, 1.0 / 3.0) : xyz_otherwise(t_Z);

	float L = ((116.0 * f_Y ) - 16.0) * 2.55;
	float a = 500.0 * ( f_X - f_Y ) + 128;
	float b = 200.0 * ( f_Y - f_Z ) + 128;

	return float3(L, a, b);
}

float lab_otherwise(float t)
{
	return (3.0 * XYZ_2_LAB_DELTA_SQUARED) * (t - (4.0 / 29.0));
}

float3 LAB_2_LinearRGB( float3 LAB, float3 ReferenceWhite)
{
	float L = LAB.x / 2.5;
	float a = LAB.y - 128;
	float b = LAB.z - 128;

	float t_y = (L + 16.0) / 116.0;
	float t_x = t_y + (a / 500.0);
	float t_z = t_y - (b / 200.0);

	float f_x = pow(t_x, 3.0);
	float f_y = pow(t_y, 3.0);
	float f_z = pow(t_z, 3.0);

	if (f_x <= XYZ_2_LAB_DELTA_CUBED)
	{
		f_x = lab_otherwise(t_x);
	}

	if (f_y <= XYZ_2_LAB_DELTA_CUBED)
	{
		f_y = lab_otherwise(t_y);
	}

	if (f_z <= XYZ_2_LAB_DELTA_CUBED)
	{
		f_z = lab_otherwise(t_z);
	}

	float X = ReferenceWhite.x * f_x;
	float Y = ReferenceWhite.y * f_y;
	float Z = ReferenceWhite.z * f_z;

	return XYZ_2_LinearRGB(float3(X, Y, Z));
}

float3 BestFitNormal_Encode(float3 normal)
{
	float p = sqrt(-normal.z * 8 + 8);
	float2 enc = normal.xy / p + 0.5f;
	float2 enc255 = enc * 255;
	float2 residual = floor(frac(enc255) * 16);
	return float3(floor(enc255), residual.x * 16 + residual.y) / 255;
}
float3 BestFitNormal_Decode(float3 normal_code)
{
	float nz = floor(normal_code.z * 255) / 16;
	normal_code.xy += float2(floor(nz) / 16, frac(nz)) / 255;
	float2 fenc = normal_code.xy * 4 - 2;
	float f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	float3 normal;
	normal.xy = fenc * g;
	normal.z = f / 2 - 1;
	return normal;
}

float4 yuv2rgb(float4 yuv)
{
	//float4x4 ycbcrToRGBTransform = float4x4(
	//	float4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
	//	float4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
	//	float4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
	//	float4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
	//);
	
	float4x4 ycbcrToRGBTransform = float4x4(
		float4(1.1640,    1.1640,  1.1640, +0.0000f),
		float4(0.0000,   -0.3920,  2.0160, +0.0000f),
		float4(1.5960,   -0.8120,  0.0000, +0.0000f),
		float4(-0.87075,  0.52925, -1.08075, +1.0000f)
	);
	float4 tmp = float4(yuv.xyz, 1.0f);
	tmp = mul(tmp, ycbcrToRGBTransform);
	return float4(tmp.xyz, yuv.w);
}

float4 srgb2rgb(float4 rgba)
{
	float4 tmp = pow(abs(rgba + 0.055) / 1.055, 2.4);
	tmp.w = rgba.w;
	return tmp;
}
