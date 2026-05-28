#include "scene_manager/octree_scene_manager.h"
#include "kernel/context.h"
#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/skeletal_mesh_component.h"
#include "math/math_utility.h"
#include "rhi/base/rhi_mesh.h"
#include <float.h>
#include <unordered_set>

SEEK_NAMESPACE_BEGIN

#define MAX_OCTREE_DEPTH 8
#define MAX_NODE_OBJECT 3

OctreeSceneManager::OctreeSceneManager(Context* context)
    : SceneManager(context)
    , m_iMaxDepth(MAX_OCTREE_DEPTH)
    , m_iMaxNodeObj(MAX_NODE_OBJECT)
    , m_bTreeDrity(true)
    , m_cRootAABBox(false)
{
}

void OctreeSceneManager::ClipScene(CameraComponent* camera)
{
    if (!m_mCachedVisibleMeshListByCamera[camera].empty())
        return;
    if (camera)
    {
        // 场景变脏时重建八叉树
        if (m_bTreeDrity || !m_pOctree)
        {
            DestructOctree(m_pOctree);
            PrepareRootBox(m_vMeshComponentList);
            m_pOctree = ConstructOctree(m_cRootAABBox, 0, m_vRootObjectList);
            m_bTreeDrity = false;
        }

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
        m_pOctree = ConstructOctree(m_cRootAABBox, 0, m_vRootObjectList);
        m_bTreeDrity = false;
    }
}

OctreeSceneManager::OctreeNodePtr OctreeSceneManager::ConstructOctree(AABBox aabb, uint32_t depth, const std::vector<MeshComponent*>& objectList)
{
    float3 fMin = aabb.Min();
    float3 fMax = aabb.Max();

    // 达到最大深度或物体数量足够少，停止划分
    if (depth >= m_iMaxDepth || objectList.size() <= m_iMaxNodeObj)
    {
        OctreeNodePtr node = MakeSharedPtr<OctreeNode>(fMin, fMax, true);
        for (auto& meshObj : objectList)
        {
            node->vObjectList.push_back(meshObj);
        }
        return node;
    }

    // 继续划分子树
    OctreeNodePtr node = MakeSharedPtr<OctreeNode>(fMin, fMax, false);
    std::vector<MeshComponent*> subObjectList[8];

    std::vector<AABBox> subTrees = SubBoxDivision(node->mBoundingBox);

    for (auto& meshObj : objectList)
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
        node->pChildren[i] = this->ConstructOctree(subTrees[i], depth + 1, subObjectList[i]);
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
        for (auto& meshObj : objectList)
        {
            // 骨骼动画的静态 AABB 不能准确反映动画后的包围盒，始终可见
            if (meshObj->GetComponentType() == ComponentType::SkeletalMesh)
            {
                auto& meshes = meshObj->GetMeshes();
                for (uint32_t i = 0; i < meshes.size(); i++)
                {
                    visible_mesh_list.push_back(std::make_pair(meshObj, i));
                }
                continue;
            }

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

        return;
    }

    for (size_t i = 0; i < 8; i++)
    {
        if (node->pChildren[i])
        {
            AABBox subAabb = node->pChildren[i]->mBoundingBox;
            if (frustum.Intersect(subAabb) != VisibleMark::No)
                this->TraverseOctree(node->pChildren[i], frustum, visible_mesh_list);
        }
    }
}

std::vector<AABBox> OctreeSceneManager::SubBoxDivision(AABBox& bbox)
{
    std::vector<AABBox> outputs;

    float3 fMin = bbox.Min();
    float3 fMax = bbox.Max();
    float3 fCenter = bbox.Center();

    float3 subMin, subMax;

    // 标准八叉树子节点顺序 (x, y, z):
    // Index 0: (0, 0, 0) - 左 下 近
    subMin = float3(fMin.x(), fMin.y(), fMin.z());
    subMax = float3(fCenter.x(), fCenter.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 1: (0, 0, 1) - 左 下 远
    subMin = float3(fMin.x(), fMin.y(), fCenter.z());
    subMax = float3(fCenter.x(), fCenter.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 2: (0, 1, 0) - 左 上 近
    subMin = float3(fMin.x(), fCenter.y(), fMin.z());
    subMax = float3(fCenter.x(), fMax.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 3: (0, 1, 1) - 左 上 远
    subMin = float3(fMin.x(), fCenter.y(), fCenter.z());
    subMax = float3(fCenter.x(), fMax.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 4: (1, 0, 0) - 右 下 近
    subMin = float3(fCenter.x(), fMin.y(), fMin.z());
    subMax = float3(fMax.x(), fCenter.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 5: (1, 0, 1) - 右 下 远
    subMin = float3(fCenter.x(), fMin.y(), fCenter.z());
    subMax = float3(fMax.x(), fCenter.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 6: (1, 1, 0) - 右 上 近
    subMin = float3(fCenter.x(), fCenter.y(), fMin.z());
    subMax = float3(fMax.x(), fMax.y(), fCenter.z());
    outputs.push_back(AABBox(subMin, subMax));

    // Index 7: (1, 1, 1) - 右 上 远
    subMin = float3(fCenter.x(), fCenter.y(), fCenter.z());
    subMax = float3(fMax.x(), fMax.y(), fMax.z());
    outputs.push_back(AABBox(subMin, subMax));

    return outputs;
}

void OctreeSceneManager::PrepareRootBox(std::vector<MeshComponent*>& meshComponentList)
{
    m_cRootAABBox = AABBox(false);
    m_vRootObjectList.clear();
    for (auto& mc : meshComponentList)
    {
        m_cRootAABBox |= mc->GetAABBoxWorld();
        m_vRootObjectList.push_back(mc);
    }
}

SEEK_NAMESPACE_END
