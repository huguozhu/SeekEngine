#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/mesh.h"
#include "rhi/base/texture.h"
#include "kernel/context.h"
#include "math/color.h"
#include <math.h>
#include "rhi/base/render_buffer.h"

#define SEEK_MACRO_FILE_UID 36     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

MeshComponent::MeshComponent(Context* context)
    :SceneComponent(context, "MeshComponent", ComponentType::Mesh)
{

}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::DelMesh(MeshPtr mesh)
{
    auto iter = std::find(m_vMeshes.begin(), m_vMeshes.end(), mesh);
    if (iter != m_vMeshes.end())
    {
        m_vMeshes.erase(iter);
    }
}

void MeshComponent::SetVisible(bool b)
{
    for (auto& mesh : m_vMeshes)
        mesh->SetVisible(b);
}

MeshPtr MeshComponent::GetMeshByIndex(size_t index)
{
    if (index >= m_vMeshes.size())
    {
        LOG_ERROR("MeshComponent::GetMeshByIndex invalid arg");
        return nullptr;
    }
    return m_vMeshes[index];
}


void MeshComponent::SetBSConfig(std::vector<BSConfig>& bsConfig)
{
    if (m_vMeshes.size() == 0)
        return;

    m_vBSConfig = bsConfig;
    auto mesh = m_vMeshes[0];
    auto& mesh_mtns = mesh->GetMorphTargetResource()._morphInfo.morph_target_names;
    size_t mesh_mtns_size = mesh_mtns.size();
    for (auto& bsConfig : m_vBSConfig)
    {
        for (size_t j = 0; j < mesh_mtns_size; j++)
        {
            if (mesh_mtns[j] == bsConfig.name)
            {
                bsConfig.nameIdx = (int32_t)j;
                break;
            }
        }
        if (bsConfig.nameIdx == -1)
            continue;

        bsConfig.relatedBSNamesIdx.clear();
        for (size_t i = 0; i < bsConfig.relatedBSNames.size(); i++)
        {
            for (size_t j = 0; j < mesh_mtns_size; j++)
            {
                if (mesh_mtns[j] == bsConfig.relatedBSNames[i])
                {
                    bsConfig.relatedBSNamesIdx.push_back((int32_t)j);
                    break;
                }
            }
        }
    }
}

SResult MeshComponent::Tick(float delta_time)
{
    for (auto mesh : m_vMeshes)
    {
        auto& mesh_mtw = mesh->GetMorphInfo().morph_target_weights;
        for (auto& bsConfig : m_vBSConfig)
        {
            if (bsConfig.nameIdx < 0 || bsConfig.nameIdx >= mesh_mtw.size())
                continue;

            switch (bsConfig.method)
            {
                case BSConfig::Method::T0:
                    if (bsConfig.coef.size() == 1)
                    {
                        float weight = bsConfig.coef[0];
                        weight = weight > 0.0f ? weight : 0.0f;
                        weight = weight < 1.0f ? weight : 1.0f;
                        mesh_mtw[bsConfig.nameIdx] = weight;
                    }
                    else
                        LOG_ERROR("MeshComponent::Tick, the size of coef error, method: %d, size: %d", bsConfig.method, bsConfig.coef.size());
                    break;
                case BSConfig::Method::T1:
                    if (bsConfig.relatedBSNamesIdx.size() == 2)
                    {
                        if (bsConfig.relatedBSNamesIdx[0] >= 0 && bsConfig.relatedBSNamesIdx[0] < mesh_mtw.size() &&
                            bsConfig.relatedBSNamesIdx[1] >= 0 && bsConfig.relatedBSNamesIdx[1] < mesh_mtw.size())
                        {
                            float weight = mesh_mtw[bsConfig.relatedBSNamesIdx[0]] * mesh_mtw[bsConfig.relatedBSNamesIdx[1]];
                            weight = weight > 0.0f ? weight : 0.0f;
                            weight = weight < 1.0f ? weight : 1.0f;
                            mesh_mtw[bsConfig.nameIdx] = weight;
                        }
                    }
                    else
                        LOG_ERROR("MeshComponent::Tick, the size of relatedBSNamesIdx error, method: %d, size: %d", bsConfig.method, bsConfig.relatedBSNamesIdx.size());
                    break;
                default:
                    LOG_ERROR("MeshComponent::Tick, wrong method type %d", bsConfig.method);
                    break;
            }
        }
    }

    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
