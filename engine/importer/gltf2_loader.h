#pragma once

#include "seek_engine.h"
#include "resource/resource_mgr.h"
#include "importer/loader.h"
#include "importer/gltf_data.h"
#include "importer/gltf_data_builder.h"
#include "importer/gltf_scene_assembler.h"
#include "components/animation_component.h"

#undef LoadImage

SEEK_NAMESPACE_BEGIN

class glTF2_Loader : public Loader
{
public:
    glTF2_Loader(Context* ctx);
    virtual ~glTF2_Loader();

    virtual SResult LoadSceneFromFile(std::string const& filePath,
                                      SceneComponentPtr& scene,
                                      std::vector<AnimationComponentPtr>& anim) override;

    virtual std::map<uint32_t, MeshComponentPtr>&           GetMeshComponentMap() override;
    virtual std::map<uint32_t, SceneComponentPtr>&          GetSceneComponentMap() override;
    virtual std::map<std::string, SceneComponentPtr>&       GetSceneComponentMapByName() override;

private:
    Context*             m_pContext;
    GltfDataBuilder      m_builder;
    GltfSceneAssembler   m_assembler;
    GltfData             m_data;
    std::string          m_filePath;
    bool                 m_isLoadSucceed;
};

SEEK_NAMESPACE_END
