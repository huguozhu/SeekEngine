/*************************************************************************************************
**
**      Copyright (C) 2022. All rights reserved.
**
**      Name                   : octree_scene_manager.h
**
**      Brief                     : octree scene manager base class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2022-07-13  Created by Anna Deng
**
**************************************************************************************************/
#pragma once

#include "scene_manager.h"
#include "math/frustum.h"

DVF_NAMESPACE_BEGIN

class OctreeSceneManager: public SceneManager
{
public:
    virtual                             ~OctreeSceneManager() {}

    DVFResult                           RenderScene(uint32_t scope = (uint32_t)RenderScope::Opacity | (uint32_t)RenderScope::Transparent /*RenderScope*/);
    
protected:
    friend class Context;
    OctreeSceneManager(Context* context);
    
    void                        ClipScene(CameraComponent* camera);
    void                        UpdateOctree();
    
    struct OctreeNode {
        CLASS_PTR(OctreeNode);
        
        AABBox                          mBoundingBox;
        OctreeNodePtr                   pChildren[8];
        bool                            bIsLeaf;
        std::vector<MeshComponent*>     vObjectList;
        
        OctreeNode()
            : mBoundingBox(true)
            , bIsLeaf(true) {}
        
        OctreeNode(float3 fMin, float3 fMax, bool isLeaf)
            : mBoundingBox(fMin, fMax)
            , bIsLeaf(isLeaf) {}
    };
    CLASS_PTR(OctreeNode);
    
private:

    OctreeNodePtr               ConstructOctree(AABBox aabb, uint32_t depth, const std::vector<MeshComponent*>& objectList);
    void                        DestructOctree(OctreeNodePtr node);
    void                        TraverseOctree(OctreeNodePtr node, Frustum const& frustum, std::vector<MeshPair>& visible_mesh_list);
    
    std::vector<AABBox>         SubBoxDivision(AABBox& bbox);
    void                        PrepareRootBox(std::vector<MeshComponent*> & meshComponentList);
    
    OctreeNodePtr                   m_pOctree;
    uint32_t                        m_iMaxDepth;
    uint32_t                        m_iMaxNodeObj;
    
    bool                            m_bTreeDrity;
    
    AABBox                          m_cRootAABBox;
    std::vector<MeshComponent*>     m_vRootObjectList;
    
};

DVF_NAMESPACE_END
