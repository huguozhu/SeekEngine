#include "importer/gltf2_loader.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"

#define SEEK_MACRO_FILE_UID 81

using namespace std;
using namespace seek_engine;

SEEK_NAMESPACE_BEGIN

glTF2_Loader::glTF2_Loader(Context* ctx)
    : m_pContext(ctx)
    , m_assembler(ctx)
{
    m_isLoadSucceed = true;
}

glTF2_Loader::~glTF2_Loader()
{
}

SResult glTF2_Loader::LoadSceneFromFile(std::string const& filePath,
                                         SceneComponentPtr& scene,
                                         vector<AnimationComponentPtr>& animations)
{
    LOG_INFO("glTF2_Loader: gltf file path: %s", filePath.c_str());
    m_filePath = filePath;

    // Phase 1: Parse glTF and build IR
    TIMER_BEG(t0);
    SResult ret = m_builder.Build(filePath, m_data);
    TIMER_END(t0, "glTF2_Loader: Phase 1 - Build IR");
    if (SEEK_CHECKFAILED(ret))
        return ret;

    // Phase 2: IR -> Engine Components
    TIMER_BEG(t1);
    ret = m_assembler.Assemble(m_data, scene, animations);
    TIMER_END(t1, "glTF2_Loader: Phase 2 - Assemble Components");
    if (SEEK_CHECKFAILED(ret))
        return ret;

    scene->SetName(filePath);
    return S_Success;
}

std::map<uint32_t, MeshComponentPtr>& glTF2_Loader::GetMeshComponentMap()
{
    return m_assembler.GetMeshComponentMap();
}

std::map<uint32_t, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMap()
{
    return m_assembler.GetSceneComponentMap();
}

std::map<std::string, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMapByName()
{
    return m_assembler.GetSceneComponentMapByName();
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID
