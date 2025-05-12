#include "scene_manager/scene_manager.h"
#include "kernel/context.h"
#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/entity.h"
#include "effect/scene_renderer.h"
#include "effect/shadow_layer.h"
#include "math/math_utility.h"
#include "math/frustum.h"
#include "math/transform.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_render_buffer.h"
#include "utils/timer.h"
#include <algorithm>

#define SEEK_MACRO_FILE_UID 39     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SceneManager::SceneManager(Context* context)
    :m_pContext(context)
{
    m_pRootComponent = MakeSharedPtr<SceneComponent>(context);
    m_pRootComponent->SetName("Scene Manager");
}

void SceneManager::PrintSceneTree(SceneComponent* component, int level)
{
    if (level > 5)
    {
        std::string prefix(level * 2, ' ');
        LOG_INFO("%s...", prefix.c_str());
        return;
    }

    std::string component_type, other_infomation;
    std::vector<std::string> wrap_informations;

    if (component->GetOwner())
        other_infomation += "[Entity:" + component->GetOwner()->GetName() + "]";

    Transform local = component->GetLocalTransform();
    if (local.Matrix() != Matrix4::Identity())
    {
        if (local.GetMatDecomposeStatus() == MatDecomposeStatus::No)
            other_infomation += "[local NoDecompose]";
        else
            other_infomation += "[local]";
    }

    switch (component->GetComponentType())
    {
        case ComponentType::Unknown:
            component_type = "Unknown";
            break;
        case ComponentType::Scene:
            component_type = "Scene";
            break;
        case ComponentType::Camera:
            component_type = "Camera";
            break;
        case ComponentType::Light:
            component_type = "Light";
            break;
        case ComponentType::Mesh:
        case ComponentType::SkeletalMesh:
        {
            //MeshComponent* mesh_component = static_cast<MeshComponent*>(component);
            //SkeletalMeshComponent* skeletal_mesh_component = nullptr;
            //if (component->GetComponentType() == ComponentType::SkeletalMesh)
            //    skeletal_mesh_component = static_cast<SkeletalMeshComponent*>(component);

            //if (skeletal_mesh_component)
            //{
            //    component_type = "SkeletalMesh";
            //    other_infomation += "[" + std::to_string(skeletal_mesh_component->GetJointCount()) + " Joints]";
            //}
            //else
            //{
            //    component_type = "Mesh";
            //}

            //size_t mesh_count = mesh_component->NumMeshes();
            //for (size_t i = 0; i < mesh_count; i++)
            //{
            //    MeshPtr pMesh = mesh_component->GetMeshByIndex(i);
            //    if (pMesh)
            //    {
            //        std::string s = "\tMesh " + std::to_string(i);
            //        if (pMesh->HasMorphTarget())
            //            s += " [" + std::to_string(pMesh->GetMorphInfo().morph_target_names.size()) + " Morph]";
            //        if (pMesh->GetMaterial())
            //            s += " [Material " + pMesh->GetMaterial()->name + "]";
            //        if(pMesh->GetSkinningJointBindSize() == SkinningJointBindSize::Four)
            //            s += " [4 Skinning Joints]";
            //        else if (pMesh->GetSkinningJointBindSize() == SkinningJointBindSize::Eight)
            //            s += " [8 Skinning Joints]";
            //        else if (pMesh->GetSkinningJointBindSize() == SkinningJointBindSize::Twelve)
            //            s += " [12 Skinning Joints]";

            //        //s += " [AABBox " + pMesh->GetAABBox().Str() + "]";
            //        wrap_informations.push_back(s);
            //    }
            //}
#if 0 // print mesh's aabox
            Matrix4 m = mesh_component->GetWorldMatrix();
            AABBox abox_m = Math::TransformAABBox(abox, m);
            other_infomation += "[aabox_m " + abox_m.Str() + "]";
            CameraComponent* camera = root->GetActiveCamera();
            if (camera)
            {
                Matrix4 v = camera->GetViewMatrix();
                Matrix4 p = camera->GetProjMatrix();
                AABBox abox_mv = Math::TransformAABBox(abox_m, v);
                AABBox abox_mvp = Math::TransformAABBox(abox_mv, p);
                other_infomation += "[aabox_mv " + abox_mv.Str() + "]";
                other_infomation += "[aabox_mvp " + abox_mvp.Str() + "]";
            }
#endif

            break;
        }
        case ComponentType::Image:
            component_type = "Image";
            break;
        case ComponentType::Animation:
            component_type = "Animation";
            break;
        case ComponentType::SkyBox:
            component_type = "SkyBox";
            break;
    }

    std::string prefix(level * 2, ' ');
    LOG_INFO("%s|%d.%s:%s %s", prefix.c_str(), level, component_type.c_str(), component->GetName().c_str(), other_infomation.c_str());
    for (auto s : wrap_informations)
    {
        LOG_INFO("%s%s", prefix.c_str(), s.c_str());
    }

    for (uint32_t i=0; i<component->NumChildren(); i++)
    {
        PrintSceneTree(component->ChildByIndex(i).get(), level + 1);
    }
}

void SceneManager::PrintTree() const
{
    PrintSceneTree(m_pRootComponent.get(), 0);
}

CameraComponent* SceneManager::GetActiveCamera()
{
    if (m_pActiveCamera)
        return m_pActiveCamera;
    if (!m_vCameraList.empty())
        return m_vCameraList[0];
    else
        return nullptr;
}

LightComponent* SceneManager::GetLightComponentByIndex(size_t index)
{
    if (index >= m_vLightList.size())
        return nullptr;
    return m_vLightList[index];
}
SResult SceneManager::Tick(float delta_time)
{
    // Step1: collect all entities
    // Step2: collect all camera and light components
    if (m_bSceneDirty)
    {
        m_bSceneDirty = false;
        m_vCurEntities.clear();
        m_vCameraList.clear();
        m_vLightList.clear();
        m_pSkyBoxComponent = nullptr;
        m_vParticleComponents.clear();
        m_vMeshComponentList.clear();
        m_vMeshList.clear();
        m_mCachedVisibleMeshListByCamera.clear();
        AddToEntityRecursion(m_pRootComponent);

        if (m_vLightList.empty())
        {
            // Use the default lights
            if (m_vDefaultLights.empty())
            {
                m_vDefaultLights.resize(3);
                m_vDefaultLights[0] = MakeSharedPtr<AmbientLightComponent>(m_pContext, "defualt light 0");
                m_vDefaultLights[1] = MakeSharedPtr<DirectionalLightComponent>(m_pContext, "defualt light 1");
                m_vDefaultLights[2] = MakeSharedPtr<DirectionalLightComponent>(m_pContext, "defualt light 2");
                m_vDefaultLights[0]->SetColor(Color(33,33,33));
                m_vDefaultLights[1]->SetDirection(float3(0, 0, 1));
                m_vDefaultLights[2]->SetDirection(float3(0, 0, -1));
                for (auto & light : m_vDefaultLights)
                    light->SetIntensity(0.5);
            }
            for (auto & light : m_vDefaultLights)
                m_vLightList.push_back(light.get());
        }
    }

    // Step3: Tick all Entities & component
    // Tick that require sequencing: here is a temporary process. TODO...
    for (Entity* entity : m_vCurEntities)
    {
        std::vector<ComponentPtr>& comps = entity->GetOwnedComponents();
        for (ComponentPtr component : comps)
        {
            if (component->GetComponentType() == ComponentType::Animation)
                SEEK_RETIF_FAIL(component->Tick(delta_time));
        }
    }
    for (Entity* entity : m_vCurEntities)
    {
        std::vector<ComponentPtr>& comps = entity->GetOwnedComponents();
        for (ComponentPtr component : comps)
        {
            if (component->GetComponentType() != ComponentType::Animation)
                SEEK_RETIF_FAIL(component->Tick(delta_time));
        }
    }
    
    TIMER_FRAME_BEG("RenderScene UpdateSkeletonMatrics");
    UpdateSkeletonMatrics();
    TIMER_FRAME_END("RenderScene UpdateSkeletonMatrics");

    // step3, clip
    CameraComponent* pActiveCamera = this->GetActiveCamera();
    ClipScene(pActiveCamera);

    return S_Success;
}

void SceneManager::AddToEntityRecursion(SceneComponentPtr scene_component)
{
    Entity* pEntity = scene_component->GetOwner();
    if (pEntity)
    {
        uint8_t isExisting = 0;
        for (size_t i = 0; i < m_vCurEntities.size(); i++)
        {
            if (pEntity == m_vCurEntities[i])
            {
                isExisting = 1;
                break;
            }
        }
        if (isExisting == 0)
            m_vCurEntities.push_back(pEntity);
    }

    if (scene_component->GetComponentType() == ComponentType::Camera)
    {
        m_vCameraList.push_back((CameraComponent*)scene_component.get());
    }

    if (scene_component->GetComponentType() == ComponentType::Light)
    {
        m_vLightList.push_back((LightComponent*)scene_component.get());
    }

    if (scene_component->GetComponentType() == ComponentType::Mesh          ||
        scene_component->GetComponentType() == ComponentType::SkeletalMesh)
    {
        MeshComponent* pMeshComponent = (MeshComponent*)scene_component.get();
        pMeshComponent->SetVisibleMark(VisibleMark::Yes);
        m_vMeshComponentList.push_back(pMeshComponent);

        auto& meshes = pMeshComponent->GetMeshes();
        for (uint32_t i=0; i<meshes.size(); i++)
        {
            m_vMeshList.push_back(std::make_pair(pMeshComponent, i));
        }
    }
    if (scene_component->GetComponentType() == ComponentType::SkyBox)
    {
        m_pSkyBoxComponent = (SkyBoxComponent*)scene_component.get();
    }
    if (scene_component->GetComponentType() == ComponentType::ParticleSystem)
    {
        m_vParticleComponents.push_back((ParticleComponent*)scene_component.get());
    }
    if (scene_component->GetComponentType() == ComponentType::WaterMark)
    {
        m_vWaterMarkComponents.push_back((WaterMarkComponent*)scene_component.get());
    }

    size_t child_num = scene_component->NumChildren();
    for (size_t i = 0; i < child_num; i++)
    {
        this->AddToEntityRecursion(scene_component->ChildByIndex(i));
    }
}

void SceneManager::UpdateSkeletonMatrics()
{
    for (MeshComponent* mesh_component : m_vMeshComponentList)
    {
        if (mesh_component->GetComponentType() == ComponentType::SkeletalMesh)
        {
            SkeletalMeshComponent* skeletal_component = (SkeletalMeshComponent*)mesh_component;
            skeletal_component->UpdateJointFinalMatrices();
        }
    }
}

void SceneManager::ClipScene(CameraComponent* camera)
{
    if (!m_mCachedVisibleMeshListByCamera[camera].empty())
        return;

    // TODO: disable clip, enable again after we can get the right AABB
    //       for skinned mesh, the initial AABB may have a large deviation, we need a better solution to calc the AABB
    
    //if (camera)
    //{    
    //    Frustum const& frustum = camera->GetFrustum();
    //    for (MeshPair & mesh_i : m_vMeshList)
    //    {
    //        AABBox world_aabb = mesh_i.first->GetMeshByIndex(mesh_i.second)->GetAABBoxWorld();
    //        if (frustum.Intersect(world_aabb) != VisibleMark::No)
    //            m_mCachedVisibleMeshListByCamera[camera].push_back(mesh_i);
    //    }
    //}
    //else
    //{
        m_mCachedVisibleMeshListByCamera[camera] = m_vMeshList;
    //}
}

class SortMeshCompare
{
private:
    float3  base;
    bool    is_closer;
public:
    SortMeshCompare(float3 _base, bool _is_closer) : base(_base), is_closer(_is_closer) {}
    bool operator () (MeshPair const& item1, MeshPair const& item2)
    {
        float dis1 = Math::Distance(base, item1.first->GetMeshByIndex(item1.second)->GetAABBoxWorld().Center());
        float dis2 = Math::Distance(base, item2.first->GetMeshByIndex(item2.second)->GetAABBoxWorld().Center());
        return is_closer ? dis1 < dis2 : dis1 > dis2;
    }
};

void SceneManager::SortMeshList(CameraComponent* camera, std::vector<MeshPair>& opacity_list, std::vector<MeshPair>& transparent_list)
{
    std::vector<MeshPair>& visible_mesh_list = m_mCachedVisibleMeshListByCamera[camera];
    for (auto mesh_i : visible_mesh_list)
    {
        auto mesh = mesh_i.first->GetMeshByIndex(mesh_i.second);
        if (!mesh->IsVisible())
            continue;

        MaterialPtr material = mesh->GetMaterial();
        if (material)
        {
            if (material->alpha_mode == AlphaMode::Opaque)
                opacity_list.push_back(mesh_i);
            else
                transparent_list.push_back(mesh_i);
        }
    }

    if (camera)
    {
        float3 position = camera->GetWorldTransform().GetTranslation();
        std::sort(opacity_list.begin(), opacity_list.end(), SortMeshCompare(position, true));
        std::sort(transparent_list.begin(), transparent_list.end(), SortMeshCompare(position, false));
    }
}

RHIRenderBufferPtr& SceneManager::GetLightInfoCBuffer()
{
    if (!m_LightInfoCBuffer)
    {
        m_LightInfoCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(LightInfo) * MAX_LIGHT, RESOURCE_FLAG_CPU_WRITE);
    }

    LightInfo lightInfo[MAX_LIGHT];
    size_t lightIndex = 0;
    for (auto light : m_vLightList)
    {
        if (!light->IsEnable())
            continue;

        LightInfo& info = lightInfo[lightIndex];

        float exposure = 1.0;
        if (m_pContext->GetLightingMode() == LightingMode::PBR)
        {
            auto camera = GetActiveCamera();
            if (camera)
                exposure = GetActiveCamera()->GetExposure() * PBR_INTENSITY_COEFF;
            else
                LOG_ERROR("no active camera");
        }

        LightType type = light->GetLightType();
        info.color = light->GetColor().ToFloat3();
        info.type = (int)type;
        info.direction = light->GetDirection();
        info.falloffRadius = light->GetFalloffRadius();
        info.posWorld = light->GetLightPos();
        info.intensity = light->GetIntensity() * exposure;
        info.castShadow = (int)m_pContext->EnableShadow() ? (int)light->CastShadow() : 0;
        info.useSoftShadow = (int)light->SoftShadow();
        info.shadowBias = light->GetShadowBias();
        if (light->GetShadowMapCamera())
        {
            CameraComponent* pCamera = light->GetShadowMapCamera();
            info.nearFarPlane = float2(pCamera->GetNearPlane(), pCamera->GetFarPlane());
        }
        if (info.castShadow)
            info.shadowMapIndex = m_pContext->SceneRendererInstance().GetShadowLayer()->GetShadowMapIndexByLightIndex(lightIndex);
        if (LightType::Spot == type)
        {
            float2 inOutCutoff = ((SpotLightComponent*)light)->GetInOutCutoff();
            info.inOutCutoff = float2(cos(inOutCutoff.x()), cos(inOutCutoff.y()));
        }

        if (LightType::Spot == type || LightType::Directional == type)
        {
            Matrix4 const& light_vp = light->GetShadowMapCamera()->GetViewProjMatrix();
            info.lightViewProj = light_vp.Transpose();
        }
        lightIndex++;
    }

    m_LightInfoCBuffer->Update(&lightInfo, sizeof(lightInfo));

    return m_LightInfoCBuffer;
}

RHIRenderBufferPtr& SceneManager::GetViewInfoCBuffer()
{
    if (!m_ViewInfoCBuffer)
    {
        m_ViewInfoCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(CameraInfo), RESOURCE_FLAG_CPU_WRITE);
    }

    // TODO: current view camera can be changed, NOW assume it won't change
    auto camera = GetActiveCamera();
    if (camera)
    {
        CameraInfo cameraInfo;
        cameraInfo.posWorld = GetActiveCamera()->GetWorldTransform().GetTranslation();
        cameraInfo.nearFarPlane = float2(GetActiveCamera()->GetNearPlane(), GetActiveCamera()->GetFarPlane());
        m_ViewInfoCBuffer->Update(&cameraInfo, sizeof(cameraInfo));
    }
    else
    {
        LOG_ERROR("no active camera");
    }

    return m_ViewInfoCBuffer;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
