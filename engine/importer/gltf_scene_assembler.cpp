#include "importer/gltf_scene_assembler.h"
#include "importer/gltf2.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "components/scene_component.h"
#include "components/mesh_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/light_component.h"
#include "components/camera_component.h"
#include "kernel/context.h"

using namespace std;
using namespace seek_engine;

#define SEEK_MACRO_FILE_UID 83

SEEK_NAMESPACE_BEGIN

SResult GltfSceneAssembler::Assemble(const GltfData& data,
                                      SceneComponentPtr& outScene,
                                      vector<AnimationComponentPtr>& outAnimations)
{
    // Clear caches to support multiple invocations
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_nodesByName.clear();

    TIMER_BEG(t0);
    BuildMaterials(data);
    BuildMeshes(data);
    BuildSkins(data);
    BuildAnimations(data, outAnimations);
    outScene = BuildScene(data);
    TIMER_END(t0, "GltfSceneAssembler: assemble");

    return S_Success;
}

// === BuildMaterials ===

void GltfSceneAssembler::BuildMaterials(const GltfData& data)
{
    for (size_t i = 0; i < data.materials.size(); i++)
    {
        BuildMaterial(data, (uint32_t)i);
    }
}

std::shared_ptr<MaterialResource> GltfSceneAssembler::BuildMaterial(
    const GltfData& data, uint32_t idx)
{
    auto it = m_materials.find(idx);
    if (it != m_materials.end())
        return it->second;

    if (idx >= data.materials.size())
        return nullptr;

    const GltfMaterial& m = data.materials[idx];
    auto mat = MakeSharedPtr<MaterialResource>();
    mat->_name = m.name;

    // Lambda: texture index -> image bitmap
    auto GetImage = [&data](uint32_t texIdx) -> BitmapBufferPtr {
        if (texIdx >= data.textures.size())
            return nullptr;
        uint32_t imgIdx = data.textures[texIdx].imageIndex;
        if (imgIdx >= data.images.size())
            return nullptr;
        return data.images[imgIdx].bitmapData;
    };

    // PBR parameters
    mat->_albedoFactor    = m.albedoFactor;
    mat->_metallicFactor  = m.metallicFactor;
    mat->_roughnessFactor = m.roughnessFactor;

    // Texture bindings
    mat->_albedoImage              = GetImage(m.albedoTextureIndex);
    mat->_metallicRoughnessImage   = GetImage(m.metallicRoughnessTextureIndex);
    mat->_normalImage              = GetImage(m.normalTextureIndex);
    mat->_normalScale              = m.normalScale;
    mat->_occlusionImage           = GetImage(m.occlusionTextureIndex);
    mat->_emmissiveImage           = GetImage(m.emissiveTextureIndex);
    mat->_emissiveFactor           = m.emissiveFactor;

    // Alpha mode
    mat->_alphaMode    = m.alphaMode;
    mat->_alphaCutoff  = m.alphaCutoff;
    mat->_doubleSided  = m.doubleSided;

    // Clearcoat (KHR_materials_clearcoat)
    if (m.hasClearcoat)
    {
        mat->_clearcoatFactor          = m.clearcoatFactor;
        mat->_clearcoatRoughnessFactor = m.clearcoatRoughnessFactor;
        mat->_clearcoatImage           = GetImage(m.clearcoatTextureIndex);
        mat->_clearcoatRoughnessImage  = GetImage(m.clearcoatRoughnessTextureIndex);
    }

    // Sheen (KHR_materials_sheen)
    if (m.hasSheen)
    {
        mat->_sheenColorFactor     = m.sheenColorFactor;
        mat->_sheenRoughnessFactor = m.sheenRoughnessFactor;
        mat->_sheenColorImage      = GetImage(m.sheenColorTextureIndex);
        mat->_sheenRoughnessImage  = GetImage(m.sheenRoughnessTextureIndex);
    }

    // IOR
    mat->_IORFactor = m.ior;

    m_materials[idx] = mat;
    return mat;
}

// === BuildMeshes + BuildPrimitive ===

void GltfSceneAssembler::BuildMeshes(const GltfData& data)
{
    for (size_t i = 0; i < data.meshes.size(); i++)
    {
        const GltfMesh& gm = data.meshes[i];
        MeshComponentPtr meshComp = MakeSharedPtr<MeshComponent>(m_pContext);

        if (!gm.name.empty())
            meshComp->SetName(gm.name);

        AABBox meshes_box;
        meshes_box.Min(float3(0.0f));
        meshes_box.Max(float3(0.0f));

        for (size_t p = 0; p < gm.primitives.size(); p++)
        {
            RHIMeshPtr rhiMesh = BuildPrimitive(data, gm.primitives[p]);
            if (rhiMesh)
            {
                meshComp->AddMesh(rhiMesh);

                // Morph target names/weights
                MorphInfo& morphInfo = rhiMesh->GetMorphTargetResource()._morphInfo;
                if (morphInfo.morph_target_names.empty() && !gm.targetNames.empty())
                    morphInfo.morph_target_names = gm.targetNames;
                if (!gm.weights.empty())
                    morphInfo.morph_target_weights = gm.weights;

                AABBox meshBox = rhiMesh->GetAABBox();
                if (p == 0)
                    meshes_box = meshBox;
                else
                    meshes_box |= meshBox;
            }
        }
        meshComp->SetAABBox(meshes_box);
        m_meshes[(uint32_t)i] = meshComp;
    }
}

RHIMeshPtr GltfSceneAssembler::BuildPrimitive(const GltfData& data,
                                               const GltfPrimitive& prim)
{
    RHIContext* rc = &m_pContext->RHIContextInstance();
    RHIMeshPtr mesh = rc->CreateMesh();
    if (!mesh)
        return mesh;

    mesh->SetTopologyType(prim.topology);
    mesh->SetSkinningJointBindSize(prim.jointBindSize);

    // Vertex attribute resource (copy since prim is const ref)
    VertexAttributeResource vertexAttrRes;
    vertexAttrRes._vertexStreams   = prim.vertexStreams;
    vertexAttrRes._vertexBuffers   = prim.vertexBuffers;
    mesh->SetVertexAttributeResource(vertexAttrRes);

    // Index buffer (copy shared_ptr to pass as non-const ref)
    if (prim.indexResource)
    {
        auto indexResCopy = prim.indexResource;
        mesh->SetIndexBufferResource(indexResCopy);
    }

    // Morph targets
    if (prim.morphTargets)
        mesh->SetMorphTargetResource(*prim.morphTargets);

    // AABB
    mesh->SetAABBox(prim.boundingBox);

    // Material
    if (prim.materialIndex < data.materials.size())
    {
        auto mat = BuildMaterial(data, prim.materialIndex);
        if (mat)
            mesh->SetMaterialResource(mat);
        else
        {
            auto defaultMat = MakeSharedPtr<MaterialResource>();
            mesh->SetMaterialResource(defaultMat);
        }
    }
    else
    {
        MaterialPtr material = MakeSharedPtr<Material>();
        mesh->SetMaterial(material);
    }

    return mesh;
}

// === BuildSkins ===

void GltfSceneAssembler::BuildSkins(const GltfData& data)
{
    // Skin joint SceneComponent binding is deferred to BuildNode.
    // BuildNode checks node->skinIndex, then:
    // 1. Recursively build all joint nodes
    // 2. Create SkeletalMeshComponent from GltfSkin data
}

// === BuildNode ===

SceneComponentPtr GltfSceneAssembler::BuildNode(const GltfData& data, uint32_t idx)
{
    auto it = m_nodes.find(idx);
    if (it != m_nodes.end())
        return it->second;

    SceneComponentPtr sc = nullptr;

    if (idx >= data.nodes.size())
        return sc;

    const GltfNode& gn = data.nodes[idx];

    // Mesh nodes use MeshComponent/SkeletalMeshComponent
    if (gn.meshIndex < data.meshes.size())
    {
        bool hasSkin = gn.skinIndex < data.skins.size();
        if (hasSkin)
        {
            const GltfSkin& skin = data.skins[gn.skinIndex];
            SkeletalMeshComponentPtr sklMeshComp =
                MakeSharedPtr<SkeletalMeshComponent>(m_pContext);

            // Recursively build joint nodes
            std::vector<SceneComponentPtr> joints;
            for (uint32_t ji : skin.jointNodeIndices)
            {
                SceneComponentPtr jointSC = BuildNode(data, ji);
                if (jointSC)
                    joints.push_back(jointSC);
            }

            sklMeshComp->SetInverseBindMatrix(skin.inverseBindMatrices);
            sklMeshComp->SetJoint(joints);

            // Skeleton root
            if (skin.skeletonNodeIndex < data.nodes.size())
            {
                SceneComponentPtr skeleton = BuildNode(data, skin.skeletonNodeIndex);
                if (skeleton)
                    sklMeshComp->SetSkeletonRoot(skeleton);
            }

            sc = sklMeshComp;
        }
        else
        {
            MeshComponentPtr meshComp = m_meshes[gn.meshIndex];
            sc = meshComp;
        }
    }
    else
        sc = MakeSharedPtr<SceneComponent>(m_pContext);

    m_nodes[idx] = sc;

    // Transform
    if (gn.hasMatrix)
        sc->SetLocalTransform(gn.matrix);
    if (gn.hasRotation)
        sc->SetLocalRotation(gn.rotation);
    if (gn.hasScale)
        sc->SetLocalScale(gn.scale);
    if (gn.hasTranslation)
        sc->SetLocalTranslation(gn.translation);

    // Name
    if (!gn.name.empty())
        sc->SetName(gn.name);

    // Recursively build child nodes
    for (uint32_t ci : gn.childIndices)
    {
        SceneComponentPtr child = BuildNode(data, ci);
        if (child)
        {
            sc->AddChild(child);
            child->SetParent(sc.get());
        }
    }

    // Name index
    if (!sc->GetName().empty())
        m_nodesByName[sc->GetName()] = sc;

    return sc;
}

// === BuildScene ===

SceneComponentPtr GltfSceneAssembler::BuildScene(const GltfData& data)
{
    SceneComponentPtr rootSC = nullptr;
    if (data.defaultSceneIndex < data.scenes.size())
    {
        const GltfScene& scene = data.scenes[data.defaultSceneIndex];
        rootSC = MakeSharedPtr<SceneComponent>(m_pContext);

        if (!scene.name.empty())
            rootSC->SetName(scene.name);

        for (uint32_t ni : scene.rootNodeIndices)
        {
            SceneComponentPtr child = BuildNode(data, ni);
            if (child)
            {
                rootSC->AddChild(child);
                child->SetParent(rootSC.get());
            }
        }
    }
    return rootSC;
}

// === BuildAnimations ===

void GltfSceneAssembler::BuildAnimations(
    const GltfData& data,
    vector<AnimationComponentPtr>& outAnimations)
{
    for (size_t animIdx = 0; animIdx < data.animations.size(); animIdx++)
    {
        const GltfAnimation& ga = data.animations[animIdx];
        AnimationComponentPtr anim = MakeSharedPtr<AnimationComponent>(m_pContext);

        if (!ga.name.empty())
            anim->SetName(ga.name);

        float startTime = 0.0f;
        float endTime   = 0.0f;

        for (size_t chIdx = 0; chIdx < ga.channels.size(); chIdx++)
        {
            const GltfAnimationChannel& ch = ga.channels[chIdx];
            SceneComponentPtr sc = BuildNode(data, ch.targetNodeIndex);
            if (!sc)
            {
                LOG_ERROR("BuildAnimations: node not found for channel");
                continue;
            }

            TransformAnimationTrackPtr tAnimTrack = nullptr;
            MorphTargetAnimationTrackPtr mtAnimTrack = nullptr;
            AnimationTrackPtr animTrack = nullptr;

            if (!ch.isMorphTarget)
            {
                tAnimTrack = anim->CreateTransformTrack();
                tAnimTrack->SetSceneComponent(sc);
                animTrack = tAnimTrack;
            }
            else
            {
                mtAnimTrack = anim->CreateMorphTargetTrack();
                mtAnimTrack->SetSceneComponent(sc);
                animTrack = mtAnimTrack;
            }

            animTrack->SetInterpolationType(ch.interpolation);

            uint32_t count = (uint32_t)ch.inputTimes.size();
            for (uint32_t k = 0; k < count; k++)
            {
                if (tAnimTrack)
                {
                    TransformKeyFramePtr kf = tAnimTrack->CreateTransformKeyFrame();
                    float time = ch.inputTimes[k];
                    kf->SetTime(time);
                    if (k == 0)
                        startTime = time;
                    endTime = time;

                    if (ch.transformType == Translate && ch.outputStride == 12)
                    {
                        const float* src = &ch.outputData[3 * k];
                        float3 translation{src[0], src[1], src[2]};
                        kf->SetTranslate(translation);
                        kf->SetOffsetTranslate(
                            translation - sc->GetLocalTransform().GetTranslation());
                    }
                    else if (ch.transformType == Rotate && ch.outputStride == 16)
                    {
                        const float* src = &ch.outputData[4 * k];
                        Quaternion qua(src[0], src[1], src[2], src[3]);
                        kf->SetRotate(qua);
                        kf->SetOffsetRotate(
                            (sc->GetLocalTransform().GetRotation().Inverse()) * qua);
                    }
                    else if (ch.transformType == Scale && ch.outputStride == 12)
                    {
                        const float* src = &ch.outputData[3 * k];
                        float3 scale{src[0], src[1], src[2]};
                        kf->SetScale(scale);
                        kf->SetOffsetScale(
                            scale / sc->GetLocalTransform().GetScale());
                    }
                    else
                        LOG_ERROR("BuildAnimations: data format not supported");
                }
                else if (mtAnimTrack)
                {
                    MorphTargetKeyFramePtr kf = mtAnimTrack->CreateMorphTargetKeyFrame();
                    float time = ch.inputTimes[k];
                    kf->SetTime(time);
                    if (k == 0)
                        startTime = time;
                    endTime = time;

                    uint32_t weightCount = ch.outputCount / count;
                    const float* wp = &ch.outputData[k * weightCount];
                    vector<float> weights(wp, wp + weightCount);
                    kf->SetWeights(weights);
                }
            }
        }

        if (anim)
        {
            AnimInfo animInfo;
            animInfo.startTime    = startTime;
            animInfo.preFrameTime = startTime;
            animInfo.endTime      = endTime;
            animInfo.state        = AnimationState::Stopped;
            animInfo.loop         = true;
            anim->AddAnimSectionInfo(animInfo);
        }
        outAnimations.push_back(anim);
    }
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID
