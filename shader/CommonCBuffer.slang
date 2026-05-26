module CommonCBuffer;


import Common;

#define COMBINE_REGISTER(type, number) type##number
#define CB_REGISTER(number) COMBINE_REGISTER(b, number)

public struct ModelInfo
{
    // model space to NDC space
    public float4x4 mvpMatrix;
    // model space to world space
    public float4x4 modelMatrix;
    // model space to view space
    public float4x4 modelViewMatrix;
    // convert normal in model space to world space
    public float4x4 normalMatrix;
};

public cbuffer ModelInfo : register(CB_REGISTER(CB_BINDING_MODEL_INFO))
{
    ModelInfo modelInfo;
};

// view information based on current camera
public struct ViewInfo
{
    // view space to NDC space
    public float4x4 viewProjMatrix;
    public float3 cameraPositionWorld;
    public float nearPlane;
    public float farPlane;
    public float3 padding;
};

public cbuffer ViewInfo : register(CB_REGISTER(CB_BINDING_VIEW_INFO))
{
    ViewInfo viewInfo;
};


public struct MaterialInfo
{
    public uint flags;
    public float3 albedoFactor;
    public float occlusionFactor;
    public float3 emissiveFactor;
    public float metallicFactor;
    public float roughnessFactor;
    public float2 padding0;
};

#define MATERIAL_ALBEDO_BIT                 0x00000001u
#define MATERIAL_NORMAL_BIT                 0x00000002u
#define MATERIAL_METALLIC_ROUGHNESS_BIT     0x00000004u
#define MATERIAL_OCCLUSION_BIT              0x00000008u
#define MATERIAL_EMISSIVE_BIT               0x00000010u

public bool HasMaterialAlbedo(in uint materialFlags)
{
    return (materialFlags & MATERIAL_ALBEDO_BIT) != 0u;
}

public bool HasMaterialNormal(in uint materialFlags)
{
    return (materialFlags & MATERIAL_NORMAL_BIT) != 0u;
}

public bool HasMaterialMetallocRoughness(in uint materialFlags)
{
    return (materialFlags & MATERIAL_METALLIC_ROUGHNESS_BIT) != 0u;
}

public bool HasMaterialOcclusion(in uint materialFlags)
{
    return (materialFlags & MATERIAL_OCCLUSION_BIT) != 0u;
}

public bool HasMaterialEmissive(in uint materialFlags)
{
    return (materialFlags & MATERIAL_EMISSIVE_BIT) != 0u;
}

public cbuffer MaterialInfo : register(CB_REGISTER(CB_BINDING_MATERIAL_INFO))
{
    MaterialInfo materialInfo;
};

public struct MorphTargetInfo
{
    public float4 morphWeights[MAX_MORPH_SIZE];
};

public cbuffer MorphTargetInfo : register(CB_REGISTER(CB_BINDING_MORPH_TARGET_INFO))
{
    MorphTargetInfo morphTargetInfo;
};
