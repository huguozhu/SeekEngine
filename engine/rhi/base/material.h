#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"

SEEK_NAMESPACE_BEGIN

class Material
{
public:
    std::string         name = "";

    TexturePtr          albedo_tex = nullptr;
    TexturePtr          normal_tex = nullptr;
    TexturePtr          normal_mask_tex = nullptr;
    TexturePtr          occlusion_tex = nullptr;
    TexturePtr          metallic_roughness_tex = nullptr;
    TexturePtr          emissive_tex = nullptr;
    //extensions
    TexturePtr          clearcoat_tex = nullptr;
    TexturePtr          clearcoat_roughness_tex = nullptr;
    TexturePtr          sheen_color_tex = nullptr;
    TexturePtr          sheen_roughness_tex = nullptr;

    float4              albedo_factor = float4(1.0);
    float               normal_scale = 1.0f;
    float3              emissive_factor = float3(0.0);
    float               metallic_factor = 1.0f;
    float               roughness_factor = 1.0f;
    //extensions
    float               clearcoat_factor = 0.0f;
    float               clearcoat_roughness_factor = 0.0f;
    float3              sheen_color_factor = float3(0.0f);
    float               sheen_roughness_factor = 0.0f;
    float               IOR_factor = 1.5f;
    float4              normal_mask_weights = float4{ 0.0, 0.0, 0.0, 0.0 };

    float               alpha_cutoff = 1.0f;
    AlphaMode           alpha_mode = AlphaMode::Opaque;
    bool                double_sided = false;
};


SEEK_NAMESPACE_END
