/*************************************************************************************************
**
**      Copyright (C) 2022. All rights reserved.
**
**      Name                   : octree_scene_manager.h
**
**      Brief                  : 八叉树场景管理器，通过八叉树空间划分加速视锥剔除
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2022-07-13  Created by Anna Deng
**                               2026-05-28  修复命名空间和编译错误，重构集成到 SEEK 架构
**
**************************************************************************************************/
#pragma once

#include "scene_manager.h"
#include "math/frustum.h"

SEEK_NAMESPACE_BEGIN

class OctreeSceneManager : public SceneManager
{
public:
    virtual ~OctreeSceneManager() {}

    struct OctreeNode
    {
        CLASS_PTR(OctreeNode);

        AABBox                      mBoundingBox;
        OctreeNodePtr               pChildren[8];
        bool                        bIsLeaf;
        std::vector<MeshComponent*> vObjectList;

        OctreeNode()
            : mBoundingBox(true)
            , bIsLeaf(true) {}

        OctreeNode(float3 fMin, float3 fMax, bool isLeaf)
            : mBoundingBox(fMin, fMax)
            , bIsLeaf(isLeaf) {}
    };

protected:
    friend class Context;
    OctreeSceneManager(Context* context);

    void ClipScene(CameraComponent* camera) override;

private:
    using OctreeNodePtr = OctreeNode::OctreeNodePtr;

    void                        UpdateOctree();
    OctreeNodePtr               ConstructOctree(AABBox aabb, uint32_t depth, const std::vector<MeshComponent*>& objectList);
    void                        DestructOctree(OctreeNodePtr node);
    void                        TraverseOctree(OctreeNodePtr node, Frustum const& frustum, std::vector<MeshPair>& visible_mesh_list);

    std::vector<AABBox>         SubBoxDivision(AABBox& bbox);
    void                        PrepareRootBox(std::vector<MeshComponent*>& meshComponentList);

    OctreeNodePtr                   m_pOctree;
    uint32_t                        m_iMaxDepth;
    uint32_t                        m_iMaxNodeObj;

    bool                            m_bTreeDrity;

    AABBox                          m_cRootAABBox;
    std::vector<MeshComponent*>     m_vRootObjectList;
};

SEEK_NAMESPACE_END
