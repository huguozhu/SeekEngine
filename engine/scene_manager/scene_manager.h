/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : scene_manager.h
**
**      Brief                     : scene manager base class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-06-11  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "kernel/kernel.h"
#include "math/rect.h"
#include "math/aabbox.h"
SEEK_NAMESPACE_BEGIN

using MeshPair = std::pair<MeshComponent*, uint32_t>;

enum class RenderScope : uint32_t
{
    Opacity     = 1UL << 1,
    Transparent = 1UL << 2,
    ALL         = 0xFFFFFFFF
};

class SceneManager
{
public:
    virtual                             ~SceneManager() {}

    static void                         PrintSceneTree(SceneComponent* component, int level = 0);
    void                                PrintTree() const;

    SceneComponentPtr                   GetRootComponent() const { return m_pRootComponent; }

    bool                                IsSceneDirty() const { return m_bSceneDirty; }
    void                                SetSceneDirty(bool v) { m_bSceneDirty = v; }

    CameraComponent*                    GetActiveCamera();
    void                                SetActiveCamera(CameraComponent* cam) { m_pActiveCamera = cam; }

    int32_t                             GetActiveLightIndex() { return m_iActiveLightIndex; }
    void                                SetActiveLightIndex(int32_t index) { m_iActiveLightIndex = index; }

    size_t                              NumLightComponent() const { return m_vLightList.size(); }
    LightComponent*                     GetLightComponentByIndex(size_t index);


    SkyBoxComponent*                    GetSkyBoxComponent() { return m_pSkyBoxComponent;}
    std::vector<ParticleComponent*>&    GetParticleComponents() { return m_vParticleComponents; }
    virtual SResult                     Tick(float delta_time);

    Rect<uint32_t>                      GetRenderRect() const { return m_RenderRect; }

    RenderBufferPtr&                    GetLightInfoCBuffer();
    RenderBufferPtr&                    GetViewInfoCBuffer();

    const std::vector<MeshPair>&        GetOpaqueMeshList() { return m_OpaqueMeshList; }
    const std::vector<MeshPair>&        GetTransparentMeshList() { return m_TransparentMeshList; }

    // Query = std::function<bool(const MeshPair&)>
    template<typename Query>
    std::vector<MeshPair> QueryMesh(Query query)
    {
        std::vector<MeshPair> outMeshes;
        CameraComponent* pActiveCamera = GetActiveCamera();
        if (!pActiveCamera)
            return outMeshes;

        std::vector<MeshPair>& allMeshes = m_mCachedVisibleMeshListByCamera[pActiveCamera];
        for (auto& mesh : allMeshes)
        {
            bool filter = query(mesh);
            if (filter)
            {
                outMeshes.emplace_back(mesh);
            }
        }
        return outMeshes;
    }

    const std::vector<MeshPair>& GetVisableMeshes()
    {
        return m_mCachedVisibleMeshListByCamera[GetActiveCamera()];
    }

protected:
    // Can only be created by class Context
    friend class Context;
    SceneManager(Context* context);

    void                                AddToEntityRecursion(SceneComponentPtr scene_component);

    void                                UpdateSkeletonMatrics();
    void                                ClipScene(CameraComponent* camera);
    void                                SortMeshList(CameraComponent* camera, std::vector<MeshPair>& opacity_list, std::vector<MeshPair>& transparent_list);

protected:
    Context*                            m_pContext = nullptr;

    Rect<uint32_t>                      m_RenderRect = Rect<uint32_t>(0, 0, 0, 0);
    /* *********************************************************************
     * Scene 3D
     * *********************************************************************/
    std::vector<LightComponentPtr>                      m_vDefaultLights;

    bool                                                m_bSceneDirty = false;
    SceneComponentPtr                                   m_pRootComponent = nullptr;

    std::vector<Entity*>                                m_vCurEntities;
    std::vector<CameraComponent*>                       m_vCameraList;
    std::vector<LightComponent*>                        m_vLightList;
    SkyBoxComponent*                                    m_pSkyBoxComponent = nullptr;
    std::vector<ParticleComponent*>                     m_vParticleComponents;
    CameraComponent*                                    m_pActiveCamera = nullptr;
    int32_t                                             m_iActiveLightIndex = -1;

    std::vector<MeshComponent*>                         m_vMeshComponentList;
    std::vector<MeshPair>                               m_vMeshList;
    std::map<CameraComponent*, std::vector<MeshPair>>   m_mCachedVisibleMeshListByCamera;
    

    /* *********************************************************************
     * Sprite 2D
     * *********************************************************************/
    
    RenderBufferPtr m_LightInfoCBuffer;
    RenderBufferPtr m_ViewInfoCBuffer;

    std::vector<MeshPair> m_OpaqueMeshList; // meshes need to draw in current rendering loop, invalid after rendering
    std::vector<MeshPair> m_TransparentMeshList;
};

SEEK_NAMESPACE_END
