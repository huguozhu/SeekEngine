#pragma once

#ifdef DVF_CPP
typedef uint32_t uint;
#endif

#define JOINT_MAX_COUNT         600
#define MAX_MORPH_SIZE          200
#define MORPH_COMPONENT_NONE    0
#define MORPH_COMPONENT_POS     1
#define MORPH_COMPONENT_NORMAL  2

#define FlippingValue -1

#define LIGHT_TYPE_AMBIENT      0
#define LIGHT_TYPE_DIRECTIONAL  1
#define LIGHT_TYPE_SPOT         2
#define LIGHT_TYPE_POINT        3


struct ModelInfo
{
    float4x4 mvpMatrix;
    float4x4 modelMatrix;
    float4x4 modelViewMatrix;
    float4x4 normalMatrix;
    float4x4 unjitterMVP;
    float4x4 preMVP;

#ifdef DVF_CPP
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

struct SkeletalJointMat
{
    float4x4 joint_mat[JOINT_MAX_COUNT];
};

struct CameraInfo
{
    float3 posWorld;
    float t0;
    float2 nearFarPlane;
    float2 t1;
};

struct MaterialParam
{
	float3 albedo;
	float  alpha;
	float3 fresnel;
	float metallic;
	float roughness;
};

struct MaterialInfo
{
    int4 hasBasicTex;
    float4 albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float occlusionFactor;
    float t1;
    float3 emissiveFactor;
    float t2;
    int4 hasPbrTex;

#ifdef DVF_CPP
    MaterialInfo()
    {
        hasBasicTex = int4{ 0, 0, 0, 0 };      // [albedoTex, normalTex, occlusionTex, emissiveTex]
        albedoFactor = float4{ 0.0, 0.0, 0.0, 0.0 };
        metallicFactor = 0.0;
        roughnessFactor = 0.0;
        occlusionFactor = 1.0;
        emissiveFactor = float3(0.0);
        hasPbrTex  = int4{ 0, 0, 0, 0 };       // [metallicRoughnessTex, irradianceTex, prefilterTex, brdf2dlutTex]
    };
#endif
};

struct LightInfo
{
    float4x4 lightViewProj;
    float3 color;
    int type;
    float3 direction;
    float falloffRadius; 
    float3 posWorld;
    float intensity;
    float2 inOutCutoff;
    int castShadow;
    int shadowMapIndex;
    int useSoftShadow;
    float shadowBias;
    float2 nearFarPlane;

#ifdef DVF_CPP
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
    }
#endif
};


// deferred shading
#define MAX_DEFERRED_LIGHTS_NUM         2048
#define MAX_DEFERRED_LIGHTS_INDICES     ((MAX_DEFERRED_LIGHTS_NUM >> 3) - 1)

#define TILE_SIZE                       16

#define NUM_CSM_LEVELS                  4

struct DeferredLightingInfo
{
    float4x4    lightVolumeMV;
    float4x4    lightVolumeInvView;    
    int         light_index_start;
    int         light_num;
    int2        t1;

#ifdef DVF_CPP
    DeferredLightingInfo()
    {
        lightVolumeMV       = float4x4::Identity();
        lightVolumeInvView  = float4x4::Identity();
        light_index_start   = 0;
        light_num           = 0;
    }
#endif
};

struct TileInfo
{
    uint light_num;
    uint light_indices[MAX_DEFERRED_LIGHTS_INDICES];
};
