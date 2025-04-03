#pragma once

#include "kernel/kernel.h"
#include "rapidjson/document.h"

SEEK_NAMESPACE_BEGIN

class Loader
{
public:
    virtual SResult LoadSceneFromFile(std::string const& filePath, SceneComponentPtr& scene, std::vector<AnimationComponentPtr>& anim) = 0;
    virtual SResult TickSceneFromJson(std::string const& jsonStr) = 0;

    virtual rapidjson::Document&                            GetModelDoc() = 0;
    virtual std::map<uint32_t, MeshComponentPtr>&           GetMeshComponentMap() = 0;
    virtual std::map<uint32_t, SceneComponentPtr>&          GetSceneComponentMap() = 0;
    virtual std::map<std::string, SceneComponentPtr>&       GetSceneComponentMapByName() = 0;
};
CLASS_DECLARE(Loader);

SEEK_NAMESPACE_END
