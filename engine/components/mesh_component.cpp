#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_render_buffer.h"
#include "effect/scene_renderer.h"
#include "effect/shadow_layer.h"
#include "kernel/context.h"
#include "math/color.h"
#include <math.h>


#define SEEK_MACRO_FILE_UID 36     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

MeshComponent::MeshComponent(Context* context)
    :SceneComponent(context, "MeshComponent", ComponentType::Mesh)
{

}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::DelMesh(RHIMeshPtr mesh)
{
    auto iter = std::find(m_vMeshes.begin(), m_vMeshes.end(), mesh);
    if (iter != m_vMeshes.end())
    {
        m_vMeshes.erase(iter);
    }
}

void MeshComponent::SetVisible(bool b)
{
    for (auto& mesh : m_vMeshes)
        mesh->SetVisible(b);
}

RHIMeshPtr MeshComponent::GetMeshByIndex(size_t index)
{
    if (index >= m_vMeshes.size())
    {
        LOG_ERROR("MeshComponent::GetMeshByIndex invalid arg");
        return nullptr;
    }
    return m_vMeshes[index];
}

SResult MeshComponent::OnRenderBegin(Technique* tech, RHIMeshPtr mesh)
{
    if (!mesh)
        return ERR_INVALID_ARG;

    SceneManager& sm = m_pContext->SceneManagerInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();
    RenderStage     stage = sr.GetCurRenderStage();

    SResult ret = S_Success;
    CameraComponent* cam = sm.GetActiveCamera();

    if (tech->HasParam("modelInfo"))
    {
        if (!m_ModelInfoCBuffer)
        {
            m_ModelInfoCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(ModelInfo), RESOURCE_FLAG_CPU_WRITE);
        }

        Matrix4 m = GetWorldMatrix();
        Matrix4 vp = Matrix4::Identity();
        Matrix4 proj = Matrix4::Identity();
        Matrix4 view = Matrix4::Identity();
        if (cam)
        {
            view = cam->GetViewMatrix();
            proj = cam->GetProjMatrix();
        }
        Matrix4 mv = m * view;
        Matrix4 mvp = mv * proj;
        ModelInfo modelInfo;
        modelInfo.mvpMatrix = mvp.Transpose();
        modelInfo.modelMatrix = m.Transpose();
        modelInfo.modelViewMatrix = mv.Transpose();
        modelInfo.normalMatrix = m.Inverse();

        m_ModelInfoCBuffer->Update(&modelInfo, sizeof(modelInfo));
        tech->SetParam("modelInfo", m_ModelInfoCBuffer);

        if (mesh->m_bUpdateFrontFaceCCW)
        {
            mesh->m_bFrontFaceCCW = m.Determinant() < 0.0f;
            mesh->m_bUpdateFrontFaceCCW = false;
        }
    }

    if (mesh->HasMorphTarget())
    {
        MorphInfo& morph_info = mesh->GetMorphInfo();
        MorphTargetType type = morph_info.morph_target_type;
        uint32_t morph_count = (uint32_t)morph_info.morph_target_weights.size();

#ifdef DEBUG
        if (0)
        {
            static int mmmm = 0;
            morph_info.morph_target_weights.clear();
            morph_info.morph_target_weights.assign(morph_count, 0);
            morph_info.morph_target_weights[(mmmm++ / 60) % morph_count] = 1;
        }
#endif

        tech->SetParam("morph_input", morph_info.render_buffer);
        tech->SetParam("MORPH_SIZE", mesh->GetMorphSizeCBuffer());
        tech->SetParam("morph_weights", mesh->GetMorphWeightsCBuffer());
    }

    switch (stage)
    {
    case RenderStage::RenderScene:
    {
        tech->SetParam("lightInfo", m_pContext->SceneManagerInstance().GetLightInfoCBuffer());
        tech->SetParam("cameraInfo", m_pContext->SceneManagerInstance().GetViewInfoCBuffer());

        this->FillMaterialParam(tech, mesh);
                
        tech->SetParam("shadowTex", m_pContext->SceneRendererInstance().GetShadowLayer()->GetFilteredShadowMap());
        tech->SetParam("cubeShadowTex", m_pContext->SceneRendererInstance().GetShadowLayer()->GetCubeShadowMap());
        
        break;
    }
    case RenderStage::GenerateCubeShadowMap:
    {
        if (!m_GenCubeShaodowCBuffer)
            m_GenCubeShaodowCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(GenCubeShadowInfo), RESOURCE_FLAG_CPU_WRITE);

        if (!cam)
        {
			LOG_ERROR("MeshComponent::OnRenderBegin GenerateCubeShadowMap cam is null");
			return ERR_INVALID_ARG;
        }
        GenCubeShadowInfo cubeInfo;
        cubeInfo.cameraFarPlane = cam->GetFarPlane();
        cubeInfo.cubeLightPos = cam->GetWorldTransform().GetTranslation();
        m_GenCubeShaodowCBuffer->Update(&cubeInfo, sizeof(GenCubeShadowInfo));
        tech->SetParam("genCubeShadowInfo", m_GenCubeShaodowCBuffer);
        break;
    }
    case RenderStage::GenerateReflectiveShadowMap:
    {
        this->FillMaterialParam(tech, mesh);

        if (!m_GenRsmLightInfoCBuffer)
        {
            m_GenRsmLightInfoCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(LightInfo), RESOURCE_FLAG_CPU_WRITE);
        }
        LightInfo lightInfo;
        uint32_t light_index = sm.GetActiveLightIndex();
        if (!cam  || light_index == -1)
        {
            LOG_ERROR("MeshComponent::OnRenderBegin GenerateCubeShadowMap cam is null");
            return ERR_INVALID_ARG;
        }
        sr.FillLightInfoByLightIndex(lightInfo, cam, light_index);
        m_GenRsmLightInfoCBuffer->Update(&lightInfo, sizeof(LightInfo));
        tech->SetParam("lightInfo", m_GenRsmLightInfoCBuffer);
        break;
    }
    case RenderStage::GenerateGBuffer:
    {
        this->FillMaterialParam(tech, mesh);
        break;
    }    
    case RenderStage::PreZ:    
    case RenderStage::GenerateShadowMap:
    {
        break;
    }
    default:
        LOG_ERROR("MeshComponent::OnRenderBegin invalid RenderStage");
    }

    SaveToPrevWorldMatrix();
    mesh->SaveToPrevMorphTargetWeights();
    return ret;
}
SResult MeshComponent::OnRenderEnd()
{
    for (auto mesh : m_vMeshes)
    {
        mesh->SetRenderState(nullptr);
    }
    return S_Success;
}
SResult MeshComponent::Render()
{
    for (uint32_t i = 0; i < m_vMeshes.size(); i++)
    {
        SEEK_RETIF_FAIL(RenderMesh(i));
    }
    return S_Success;
}
SResult MeshComponent::RenderMesh(uint32_t i)
{
    RHIMeshPtr pMesh = m_vMeshes[i];
    if (!pMesh->IsVisible())
        return S_Success;

    Technique* tech;
    SEEK_RETIF_FAIL(m_pContext->SceneRendererInstance().GetEffectTechniqueToRender(pMesh, &tech));
    SEEK_RETIF_FAIL(this->OnRenderBegin(tech, pMesh));
    SEEK_RETIF_FAIL(tech->Render(pMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());

    return S_Success;
}

void MeshComponent::FillMaterialParam(Technique* tech, RHIMeshPtr& pMesh)
{
    MaterialPtr pMaterial = pMesh->GetMaterial();
    RHIGpuBufferPtr pMaterialCBuffer = pMesh->GetMaterialInfoCBuffer();

    SceneManager& sm = m_pContext->SceneManagerInstance();

    tech->SetParam("albedoTex", pMaterial->albedo_tex);
    tech->SetParam("metallicRoughnessTex", pMaterial->metallic_roughness_tex);
    tech->SetParam("normalTex", pMaterial->normal_tex);
    tech->SetParam("normalMaskTex", pMaterial->normal_mask_tex);
    tech->SetParam("occlusionTex", pMaterial->occlusion_tex);

    bool has_precomputed_ibl = m_pContext->HasPrecomputedIBL();
    auto& specular = m_pContext->GetIBLSpecularTexture();
    auto& diffuse = m_pContext->GetIBLDiffuseTexture();
    auto& brdf_tex = m_pContext->GetIBLBrdfTex();
    uint32_t specular_mipmaps = 1;
    if (specular)
        specular_mipmaps = specular->Descriptor().num_mips;
    if (has_precomputed_ibl)
    {
        tech->SetParam("prefilterTex", specular);
        tech->SetParam("irradianceTex", diffuse);
        tech->SetParam("brdf2dlutTex", brdf_tex);
    }

    MaterialInfo materialInfo;
    if (pMaterial)
    {
        materialInfo.albedoFactor = pMaterial->albedo_factor;
        materialInfo.metallicFactor = pMaterial->metallic_factor;
        materialInfo.roughnessFactor = pMaterial->roughness_factor;
        materialInfo.emissiveFactor = pMaterial->emissive_factor;
        materialInfo.normalScale = pMaterial->normal_scale;
        materialInfo.normalMaskWeights = pMaterial->normal_mask_weights;
        materialInfo.occlusionFactor = 1.0;

        int32_t has_tex_albedo = pMaterial->albedo_tex ? 1 : 0;
        int32_t has_tex_normal = pMaterial->normal_tex ? 1 : 0;
        int32_t has_tex_occlusion = pMaterial->occlusion_tex ? 1 : 0;
        int32_t has_tex_emissive = pMaterial->emissive_tex ? 1 : 0;

        int32_t has_tex_metallic_roughness = pMaterial->metallic_roughness_tex ? 1 : 0;

        if (!m_pContext->EnableAmbientOcclusion())
            has_tex_occlusion = 0;

        materialInfo.hasBasicTex = int4{ has_tex_albedo, has_tex_normal, has_tex_occlusion, has_tex_emissive };
        materialInfo.hasPbrTex = int4{ has_tex_metallic_roughness, 0, 0, 0 };
        materialInfo.options.z() = (int)has_precomputed_ibl;
        materialInfo.options.w() = (int)specular_mipmaps;
    }

    pMaterialCBuffer->Update(&materialInfo, sizeof(materialInfo));
    tech->SetParam("materialInfo", pMaterialCBuffer);
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
