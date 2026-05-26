module Common;


#ifdef SEEK_CPP
typedef uint32_t uint;
#endif


public static const int JOINT_MAX_COUNT = 600;
public static const int MAX_MORPH_SIZE = 200;

public static const int MORPH_COMPONENT_NONE = 0;
public static const int MORPH_COMPONENT_POS = 1;
public static const int MORPH_COMPONENT_NORMAL = 2;

public static const int FlippingValue = -1;

public static const int LIGHT_TYPE_AMBIENT = 0;
public static const int LIGHT_TYPE_DIRECTIONAL = 1;
public static const int LIGHT_TYPE_SPOT = 2;
public static const int LIGHT_TYPE_POINT = 3;


public static const int CB_BINDING_MODEL_INFO = 0;
public static const int CB_BINDING_VIEW_INFO = 1;
public static const int CB_BINDING_LIGHT_INFO = 2;
public static const int CB_BINDING_MATERIAL_INFO = 3;
public static const int CB_BINDING_MORPH_TARGET_INFO = 4;

public static const int CB_BINDING_FIXED_COUNT = 5;

public struct ModelInfo
{
    public float4x4 mvpMatrix;
    public float4x4 modelMatrix;
    public float4x4 modelViewMatrix;
    public float4x4 normalMatrix;
	public float4x4 unjitterMVP;
    public float4x4 preMVP;

#ifdef SEEK_CPP
    ModelInfo()
    {
        mvpMatrix       = float4x4::Identity();
        modelMatrix     = float4x4::Identity();
        modelViewMatrix = float4x4::Identity();
        normalMatrix    = float4x4::Identity();
		unjitterMVP     = float4x4::Identity();
        preMVP          = float4x4::Identity();
    }
#endif
};

public struct SkeletalJointMat
{
    public float4x4 joint_mat[JOINT_MAX_COUNT];
};

public struct CameraInfo
{
    public float3 posWorld;
    public float t0;
    public float nearPlane;
    public float farPlane;
    public float2 t1;
};

public struct MaterialParam
{
	public float3 albedo;
	public float  alpha;
	public float3 fresnel;
	public float metallic;
	public float roughness;
};

public struct MaterialInfo
{
    public int4 hasBasicTex;
    public float4 albedoFactor;
    public float metallicFactor;
    public float roughnessFactor;
    public float occlusionFactor;
    public float normalScale;
    public float3 emissiveFactor;
    public float t2;
    public float4 normalMaskWeights;
    public int4 hasPbrTex;
    public int4 options; // x: sss, y: eye glint, z: IBL, w: specular_mipmaps

#ifdef SEEK_CPP
    MaterialInfo()
    {
        hasBasicTex = int4{ 0, 0, 0, 0 };      // [albedoTex, normalTex, occlusionTex, emissiveTex]
        albedoFactor = float4{ 0.0, 0.0, 0.0, 0.0 };
        metallicFactor = 0.0;
        roughnessFactor = 0.0;
        occlusionFactor = 1.0;
        normalScale = 1.0;
        emissiveFactor = float3(0.0);
        normalMaskWeights = float4{ 0.0, 0.0, 0.0, 0.0 };
        hasPbrTex  = int4{ 0, 0, 0, 0 };       // [metallicRoughnessTex, irradianceTex, prefilterTex, normalMaskTex]
        options = int4{ 0, 0, 0, 0 };
    };
#endif
};

public struct LightInfo
{
    public float4x4 lightViewProj;
    public float3 color;
    public int type;
    public float3 direction;
    public float falloffRadius; 
    public float3 posWorld;
    public float intensity;
    public float2 inOutCutoff;
    public int castShadow;
    public int shadowMapIndex;
    public int useSoftShadow;
    public float shadowBias;
    public float2 nearFarPlane;

#ifdef SEEK_CPP
    LightInfo()
    {
        lightViewProj = float4x4::Identity();
        color = float3(0.0f, 0.0f, 0.0f);
        type = 0xffffffff;
        direction = float3(0.0f, 0.0f, 0.0f);
        falloffRadius = 0.0f;
        posWorld = float3(0.0f, 0.0f, 0.0f);
        intensity = 0.0f;
        inOutCutoff = float2(0.0f, 0.0f);
        castShadow = 0;
        shadowMapIndex = 0;
        useSoftShadow = 0;
        shadowBias = 0.0;
		nearFarPlane = float2(0.0f, 0.0f);
    }
#endif
};

public static const int MAX_DEFERRED_LIGHTS_NUM = 1024;
public static const int MAX_DEFERRED_LIGHTS_INDICES = ((MAX_DEFERRED_LIGHTS_NUM >> 3) - 1);

public static const int TILE_SIZE = 16;

public static const int NUM_CSM_LEVELS = 4;

public struct DeferredLightingInfo
{
    public float4x4    lightVolumeMV;
    public float4x4    lightVolumeInvView;
    public int         light_index_start;
    public int         light_num;
    public int2        t1;

#ifdef DVF_CPP
    DeferredLightingInfo()
    {
        lightVolumeMV = float4x4::Identity();
        lightVolumeInvView = float4x4::Identity();
        light_index_start = 0;
        light_num = 0;
    }
#endif
};

public struct LightCullingInfo
{
    public float4x4 view_matrix;
    public float4x4 proj_matrix;
    public float2 frame_size;
    public int light_index_start;
    public int light_num;
};

public struct TileInfo
{
    public uint light_num;
    public uint light_indices[MAX_DEFERRED_LIGHTS_INDICES];
};

public struct GenCubeShadowInfo
{
    public float cameraFarPlane;
    public float3 cubeLightPos;
};



public static const float PI 					= 3.1415926;
public static const float3 RGB_TO_LUM			= float3(0.2126, 0.7152, 0.0722);

public SamplerState linear_sampler;
public SamplerState point_sampler;
public SamplerState shadow_map_sampler;

public float4x4 MatrixInverse(float4x4 mat)
{
    public float n11 = mat[0][0], n12 = mat[1][0], n13 = mat[2][0], n14 = mat[3][0];
    public float n21 = mat[0][1], n22 = mat[1][1], n23 = mat[2][1], n24 = mat[3][1];
    public float n31 = mat[0][2], n32 = mat[1][2], n33 = mat[2][2], n34 = mat[3][2];
    public float n41 = mat[0][3], n42 = mat[1][3], n43 = mat[2][3], n44 = mat[3][3];

    public float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    public float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    public float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    public float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    public float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    public float idet = 1.0f / det;

    public float r00 = t11 * idet;
    public float r01 = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    public float r02 = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    public float r03 = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    public float r10 = t12 * idet;
    public float r11 = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    public float r12 = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    public float r13 = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    public float r20 = t13 * idet;
    public float r21 = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    public float r22 = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    public float r23 = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    public float r30 = t14 * idet;
    public float r31 = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    public float r32 = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    public float r33 = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;
	
	public float4x4 ret = float4x4(r00, r01, r02, r03, r10, r11, r12, r13, r20, r21, r22, r23, r30, r31, r32, r33);

    return ret;
}

public float2 TexCoordFromPosNDC(float3 posNDC)
{
	public float2 texCoord = posNDC.xy * 0.5f;
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
	texCoord.y *= 1;
#else
    texCoord.y *= -1;
#endif	
	texCoord += 0.5f;
	return texCoord;
}

public float2 TexCoordToPosNDC(float2 texCoord)
{
	public float2 posNDC = texCoord - 0.5f;
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
public float ViewSpaceDepth(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return (2.0 * n * f) / (f + n - z * (f - n));
#else
	return (n * f) / (f - z	* (f - n));
#endif
}

// "depth buffer Z-value" --> "linear [0..1] range (near plane to far plane)"
public float LinearizeDepth(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return n * (z + 1.0) / (f + n - z * (f - n));
#else
	return z / (f - z * (f - n));
#endif
}

// "depth buffer Z-value" --> "linear [0..1] range (eye to far plane)"
public float LinearizeDepth2(float z, float n, float f)
{
#if defined(SEEK_GLSL) || defined(SEEK_GLSL_ES)
    return (2 * n) / (f + n - z * (f - n));
#else
	return n / (f - z * (f - n));
#endif
}

public float LinearDetpthToNonLinear(float depth, float cam_near, float cam_far)
{
	public float q = cam_far / (cam_far - cam_near);
	return (depth * q - cam_near * q) / depth;
}

public static const float3x3 XYZ_2_LinearRGB_MAT =
{
	 3.2409699419, -1.5373831776, -0.4986107603,
	-0.9692436363,  1.8759675015,  0.0415550574,
	 0.0556300797, -0.2039769589,  1.0569715142,
};

public static const float3x3 LinearRGB_2_XYZ_MAT =
{
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041,
};

public float3 LinearRGB_2_XYZ( float3 LinearRGB )
{
	return mul(LinearRGB_2_XYZ_MAT, LinearRGB);
}

public float3 XYZ_2_LinearRGB( float3 XYZ )
{
	return mul(XYZ_2_LinearRGB_MAT, XYZ);
}

public static const float3 XYZ_WHITE_REF_D65 = float3(0.95047, 1.000, 1.08883);
public static const float3 XYZ_WHITE_REF_D50 = float3(0.966797, 1.000, 0.825188);
public static const float XYZ_2_LAB_DELTA_SQUARED = 0.04280618311; // (6/29)^3
public static const float XYZ_2_LAB_DELTA_CUBED = 0.00885645167; // (6/29)^3

public float xyz_otherwise(float t)
{
	return (t / (3.0 * XYZ_2_LAB_DELTA_SQUARED)) + 4.0 / 29.0;
}

public float3 LinearRGB_2_LAB( float3 LinearRGB, float3 ReferenceWhite )
{
	public float3 XYZ = LinearRGB_2_XYZ(LinearRGB);

	public float t_X = XYZ.x / ReferenceWhite.x;
	public float t_Y = XYZ.y / ReferenceWhite.y;
	public float t_Z = XYZ.z / ReferenceWhite.z;

	public float f_X = (t_X > XYZ_2_LAB_DELTA_CUBED) ? pow(t_X, 1.0 / 3.0) : xyz_otherwise(t_X);
	public float f_Y = (t_Y > XYZ_2_LAB_DELTA_CUBED) ? pow(t_Y, 1.0 / 3.0) : xyz_otherwise(t_Y);
	public float f_Z = (t_Z > XYZ_2_LAB_DELTA_CUBED) ? pow(t_Z, 1.0 / 3.0) : xyz_otherwise(t_Z);

	public float L = ((116.0 * f_Y ) - 16.0) * 2.55;
	public float a = 500.0 * ( f_X - f_Y ) + 128;
	public float b = 200.0 * ( f_Y - f_Z ) + 128;

	return float3(L, a, b);
}

public float lab_otherwise(float t)
{
	return (3.0 * XYZ_2_LAB_DELTA_SQUARED) * (t - (4.0 / 29.0));
}

public float3 LAB_2_LinearRGB( float3 LAB, float3 ReferenceWhite)
{
	public float L = LAB.x / 2.5;
	public float a = LAB.y - 128;
	public float b = LAB.z - 128;

	public float t_y = (L + 16.0) / 116.0;
	public float t_x = t_y + (a / 500.0);
	public float t_z = t_y - (b / 200.0);

	public float f_x = pow(t_x, 3.0);
	public float f_y = pow(t_y, 3.0);
	public float f_z = pow(t_z, 3.0);

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

	public float X = ReferenceWhite.x * f_x;
	public float Y = ReferenceWhite.y * f_y;
	public float Z = ReferenceWhite.z * f_z;

	return XYZ_2_LinearRGB(float3(X, Y, Z));
}

public float3 BestFitNormal_Encode(float3 normal)
{
	public float p = sqrt(-normal.z * 8 + 8);
	public float2 enc = normal.xy / p + 0.5f;
	public float2 enc255 = enc * 255;
	public float2 residual = floor(frac(enc255) * 16);
	return float3(floor(enc255), residual.x * 16 + residual.y) / 255;
}
public float3 BestFitNormal_Decode(float3 normal_code)
{
	public float nz = floor(normal_code.z * 255) / 16;
	normal_code.xy += float2(floor(nz) / 16, frac(nz)) / 255;
	public float2 fenc = normal_code.xy * 4 - 2;
	public float f = dot(fenc, fenc);
	public float g = sqrt(1 - f / 4);
	public float3 normal;
	normal.xy = fenc * g;
	normal.z = f / 2 - 1;
	return normal;
}

public float4 yuv2rgb(float4 yuv)
{
	//float4x4 ycbcrToRGBTransform = float4x4(
	//	float4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
	//	float4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
	//	float4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
	//	float4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
	//);
	
	public float4x4 ycbcrToRGBTransform = float4x4(
		float4(1.1640,    1.1640,  1.1640, +0.0000f),
		float4(0.0000,   -0.3920,  2.0160, +0.0000f),
		float4(1.5960,   -0.8120,  0.0000, +0.0000f),
		float4(-0.87075,  0.52925, -1.08075, +1.0000f)
	);
	public float4 tmp = float4(yuv.xyz, 1.0f);
	tmp = mul(tmp, ycbcrToRGBTransform);
	return float4(tmp.xyz, yuv.w);
}

public float4 srgb2rgb(float4 rgba)
{
	public float4 tmp = pow(abs(rgba + 0.055) / 1.055, 2.4);
	tmp.w = rgba.w;
	return tmp;
}
