module CommonTypes;


#ifdef SEEK_CPP
typedef uint32_t uint;
#endif

// 数组大小的常量必须在模块内可见
public static const int JOINT_MAX_COUNT = 600;
public static const int MAX_MORPH_SIZE = 200;
public static const int MAX_DEFERRED_LIGHTS_NUM = 1024;
public static const int MAX_DEFERRED_LIGHTS_INDICES = ((1024 >> 3) - 1);

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
        public intensity = 0.0f;
        inOutCutoff = float2(0.0f, 0.0f);
        castShadow = 0;
        shadowMapIndex = 0;
        useSoftShadow = 0;
        shadowBias = 0.0;
		nearFarPlane = float2(0.0f, 0.0f);
    }
#endif
};

// deferred shading
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
