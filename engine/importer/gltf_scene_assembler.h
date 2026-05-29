#pragma once

#include "importer/gltf_data.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

class GltfSceneAssembler
{
public:
    explicit GltfSceneAssembler(Context* ctx)
        : m_pContext(ctx)
    {}

    SResult Assemble(const GltfData& data,
                     SceneComponentPtr& outScene,
                     std::vector<AnimationComponentPtr>& outAnimations);

    std::map<uint32_t, MeshComponentPtr>&           GetMeshComponentMap()   { return m_meshes; }
    std::map<uint32_t, SceneComponentPtr>&          GetSceneComponentMap()  { return m_nodes; }
    std::map<std::string, SceneComponentPtr>&       GetSceneComponentMapByName() { return m_nodesByName; }

private:
    void BuildMaterials(const GltfData& data);
    void BuildMeshes(const GltfData& data);
    void BuildSkins(const GltfData& data);
    SceneComponentPtr BuildNode(const GltfData& data, uint32_t idx);
    void BuildAnimations(const GltfData& data,
                         std::vector<AnimationComponentPtr>& outAnimations);
    SceneComponentPtr BuildScene(const GltfData& data);

    std::shared_ptr<MaterialResource> BuildMaterial(const GltfData& data, uint32_t idx);
    RHIMeshPtr BuildPrimitive(const GltfData& data, const GltfPrimitive& prim);

    Context* m_pContext;

    std::map<uint32_t, std::shared_ptr<MaterialResource>> m_materials;
    std::map<uint32_t, MeshComponentPtr>                  m_meshes;
    std::map<uint32_t, SceneComponentPtr>                 m_nodes;
    std::map<std::string, SceneComponentPtr>              m_nodesByName;
};

SEEK_NAMESPACE_END
