#include "scene_manager/octree_scene_manager.h"
#include "kernel/context.h"
#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/image_component.h"
#include "components/light_component.h"
#include "components/entity.h"
#include <float.h>

DVF_NAMESPACE_BEGIN

#define MAX_OCTREE_DEPTH 8
#define MAX_NODE_OBJECT 3

OctreeSceneManager::OctreeSceneManager(Context* context)
    :SceneManager(context)
    ,m_iMaxDepth(MAX_OCTREE_DEPTH)
    ,m_iMaxNodeObj(MAX_NODE_OBJECT)
    ,m_bTreeDrity(true)
    ,m_cRootAABBox(false)
{
}

DVFResult OctreeSceneManager::RenderScene(uint32_t scope)
{
    if (m_vMeshList.empty())
    {
        return DVF_Success;
    }

    std::vector<MeshPair> opacity_mesh_list;
    std::vector<MeshPair> transparent_mesh_list;

    // step1, Update skeleton matrics
    UpdateSkeletonMatrics();

    // step2, Update aabbox of all meshes
    UpdateMeshAABBox();
    
    // step2extend, Update Octree for meshes
    UpdateOctree();
    
    // step3, clip
    CameraComponent* pActiveCamera = this->GetActiveCamera();
    ClipScene(pActiveCamera);
    if (m_mCachedVisibleMeshListByCamera[pActiveCamera].empty())
    {
        LOG_INFO("SceneManager::RenderScene() MeshCnt:%d VisibleMeshCnt=0, skip render", m_vMeshList.size());
        return DVF_Success;
    }

    // step4, sort
    SortMeshList(pActiveCamera, opacity_mesh_list, transparent_mesh_list);

    // step5, render opaticy
    if (scope & (uint32_t)RenderScope::Opacity)
    {
        TIMER_FRAME_BEG("RenderScene RenderMesh Opaticy");
        for (MeshPair& mesh_id : opacity_mesh_list)
        {
            DVF_RETIF_FAIL(mesh_id.first->RenderMesh(mesh_id.second));
        }
        TIMER_FRAME_END("RenderScene RenderMesh Opaticy");
    }

    // step6, render transparent
    if (scope & (uint32_t)RenderScope::Transparent)
    {
        if (m_pContext->SceneRendererInstance().GetCurRenderStage() == RenderStage::RenderScene)
        {
            TIMER_FRAME_BEG("RenderScene RenderMesh Transparent");
            for (MeshPair& mesh_id : transparent_mesh_list)
            {
                DVF_RETIF_FAIL(mesh_id.first->RenderMesh(mesh_id.second));
            }
            TIMER_FRAME_END("RenderScene RenderMesh Transparent");
        }
    }
    return DVF_Success;
}

void OctreeSceneManager::ClipScene(CameraComponent* camera)
{
    if (!m_mCachedVisibleMeshListByCamera[camera].empty())
        return;
    if (camera)
    {
        Frustum const& frustum = camera->GetFrustum();
        TraverseOctree(m_pOctree, frustum, m_mCachedVisibleMeshListByCamera[camera]);
    }
    else
    {
        m_mCachedVisibleMeshListByCamera[camera] = m_vMeshList;
    }
}

void OctreeSceneManager::UpdateOctree()
{
    if (m_bTreeDrity)
    {
        DestructOctree(m_pOctree);
        PrepareRootBox(m_vMeshComponentList);
        m_pOctree = ConstructOctree(m_cRootAABBox, 0, m_vMeshComponentList);
        m_bTreeDrity = false;
    }
}

OctreeSceneManager::OctreeNodePtr OctreeSceneManager::ConstructOctree(AABBox aabb, uint32_t depth, const std::vector<MeshComponent*>& objectList)
{
    float3 fMin = aabb.Min();
    float3 fMax = aabb.Max();
    
    // check if conditions met, stop dividing
    if (depth >= m_iMaxDepth || objectList.size() <= m_iMaxNodeObj)
    {
        OctreeNodePtr node = MakeSharedPtr<OctreeNode>(fMin, fMax, true);
        
        for(auto &meshObj : objectList)
        {
            node->vObjectList.push_back(meshObj);
            //if (m_pContext->IsDebug())
            //    LOG_DEBUG("OctreeSceneManager::ConstructOctree \n\t TreeNode %p || TreeBox %s || MeshObj %p", node.get(), aabb.Str().c_str(), meshObj);

        }
        return node;
    }
    
    // continue dividing sub-tree
    OctreeNodePtr node = MakeUniquePtr<OctreeNode>(fMin, fMax, false);
    std::vector<MeshComponent*> subObjectList[8];
    
    AABBox bbox = node->mBoundingBox;
    std::vector<AABBox> subTrees = SubBoxDivision(node->mBoundingBox);
    
    for(auto &meshObj : objectList)
    {
        AABBox mesh_aabb = meshObj->GetAABBoxWorld();
        
        for (size_t j = 0; j < 8; j++)
        {
            if (Math::Intersect_AABBox_AABBox(subTrees[j], mesh_aabb))
            {
                subObjectList[j].push_back(meshObj);
            }
        }
    }
    
    for (size_t i = 0; i < 8; i++)
    {
        node->pChildren[i] = this->ConstructOctree(subTrees[i], depth+1, subObjectList[i]);
    }
    
    return node;
   
}

void OctreeSceneManager::DestructOctree(OctreeNodePtr node)
{
    if (!node)
        return;
    
    for (size_t i = 0; i < 8; i++)
        DestructOctree(node->pChildren[i]);
    
    node.reset();
}

void OctreeSceneManager::TraverseOctree(OctreeNodePtr node, Frustum const& frustum, std::vector<MeshPair>& visible_mesh_list)
{
    if (!node)
        return;
    
    AABBox aabb = node->mBoundingBox;
    if (frustum.Intersect(aabb) == VisibleMark::No)
        return;
    
    if (node->bIsLeaf)
    {
        std::vector<MeshComponent*>& objectList = node->vObjectList;
        for(auto &meshObj : objectList)
        {
            if(std::find_if(visible_mesh_list.begin(), visible_mesh_list.end(), [&](MeshPair const & ref){
                return ref.first == meshObj;
            }) == visible_mesh_list.end())
            {
            
                auto& meshes = meshObj->GetMeshes();
                for (uint32_t i = 0; i < meshes.size(); i++)
                {
                    AABBox mesh_aabb = meshes[i]->GetAABBoxWorld();
                    if (frustum.Intersect(mesh_aabb) != VisibleMark::No)
                    {
                        visible_mesh_list.push_back(std::make_pair(meshObj, i));
                    }
                }
            }
        }
        
        return;
    }
    
    for (size_t i = 0; i < 8; i++)
    {
        AABBox subAabb = node->pChildren[i]->mBoundingBox;
        if (frustum.Intersect(subAabb) != VisibleMark::No)
            this->TraverseOctree(node->pChildren[i], frustum, visible_mesh_list);
    }

}

std::vector<AABBox> OctreeSceneManager::SubBoxDivision(AABBox& bbox)
{
    std::vector<AABBox> outputs;
    
    float3 fMin = bbox.Min();
    float3 fMax = bbox.Max();
    float3 fCenter = bbox.Center();

    float3 subMin, subMax;
    
    // Index 0 0 0
    subMin = fMin;
    subMax = fCenter;
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 0 0 1
    subMin = float3(fMin.x(), fMin.y(), fCenter.z());
    subMax = float3(fCenter.x(), fCenter.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 0 1 0
    subMin = float3(fMin.x(), fCenter.y(), fMin.z());
    subMax = float3(fCenter.x(), fMax.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 0 1 1
    subMin = float3(fMin.x(), fCenter.y(), fCenter.z());
    subMax = float3(fCenter.x(), fMax.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 1 1 0
    subMin = float3(fCenter.x(), fCenter.y(), fMin.z());
    subMax = float3(fMax.x(), fMax.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 1 1 1
    subMin = float3(fCenter.x(), fCenter.y(), fCenter.z());
    subMax = float3(fMax.x(), fMax.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 1 0 0
    subMin = float3(fCenter.x(), fMin.y(), fMin.z());
    subMax = float3(fMax.x(), fCenter.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    // Index 1 0 1
    subMin = float3(fCenter.x(), fMin.y(), fCenter.z());
    subMax = float3(fMax.x(), fCenter.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));
    
    return outputs;
}

void OctreeSceneManager::PrepareRootBox(std::vector<MeshComponent*> & meshComponentList)
{
    for (auto & mc : meshComponentList)
    {
        m_cRootAABBox |= mc->GetAABBoxWorld();
    }
}

DVF_NAMESPACE_END

