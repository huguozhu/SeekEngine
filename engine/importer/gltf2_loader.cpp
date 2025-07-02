#include "gltf2_loader.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/zbase64.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/math_utility.h"
#include <fstream>
#include "zlib.h"

using namespace std;
using namespace seek_engine;
using namespace rapidjson;

#define SEEK_MACRO_FILE_UID 81     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define RETURN_IF_FOUND(map, key) \
    { \
        auto i = (map).find(key); \
        if (i != (map).end()) { \
            return i->second; \
        } \
    }

#define INIT_AND_RETURN_IF_FOUND(vec, objname, index)       \
    if (vec.empty())                                        \
    {                                                       \
        if (m_Doc.HasMember(objname))                       \
        {                                                   \
            Value& v = m_Doc[objname];                      \
            if (v.IsArray())                                \
                vec.resize(v.Size());                       \
        }                                                   \
    }                                                       \
    if (index >= vec.size())                                \
        return nullptr;                                     \
    if (vec[index])                                         \
        return vec[index];

glTF2_Loader::glTF2_Loader(Context* ctx)
    : m_pContext(ctx)
{
    m_isLoadSucceed = true;
    m_isGLBFormat = false;
}

glTF2_Loader::~glTF2_Loader()
{

}

SResult glTF2_Loader::LoadSceneFromFile(std::string const& filePath, SceneComponentPtr& scene, vector<AnimationComponentPtr>& animations)
{
    SResult ret = S_Success;
    LOG_INFO("glTF2_Loader: gltf file path: %s", filePath.c_str());
    
    TIMER_BEG(t0);
    auto fileRes = MakeSharedPtr<FileResource>(filePath);
    if (!fileRes->IsAvailable())
    {
        LOG_ERROR("glTF2_Loader: read file fail", filePath.c_str());
        return ret;
    }
    TIMER_END(t0, "glTF2_Loader: read file");

    m_filePath = filePath;
    uint8_t* JSONData = (uint8_t*)fileRes->_data;
    uint32_t JSONLength = (uint32_t)fileRes->_size;

    std::shared_ptr<BufferResource> gltfBufferRes = MakeSharedPtr<BufferResource>();
    if (((int*)JSONData)[0] == 0xFFFFFFFF)
    {
        TIMER_BEG(t1);
        size_t unzipDataSize = ((int*)JSONData)[1];
        std::shared_ptr<uint8_t> unzipData{ new uint8_t[unzipDataSize], default_array_deleter<uint8_t>() };
        int ret = uncompress(unzipData.get(), (uLongf*)&unzipDataSize, JSONData + 8, JSONLength - 8);
        if (ret != Z_OK)
        {
            LOG_ERROR("glTF2_Loader: uncompress file fail");
            return ERR_INVALID_ARG;
        }
        JSONData = unzipData.get();
        JSONLength = (uint32_t)unzipDataSize;
        gltfBufferRes->_data = JSONData;
        gltfBufferRes->_size = JSONLength;
        gltfBufferRes->_uninitializer = [unzipData](IResource*) mutable {
            unzipData.reset();
        };
        TIMER_END(t1, "glTF2_Loader: uncompress file");
    }
    else
    {
        gltfBufferRes->_data = JSONData;
        gltfBufferRes->_size = JSONLength;
        gltfBufferRes->RetainBackendResource(fileRes);
    }

    m_isGLBFormat = false;
    gltf::GLBHeader* glbHeader = (gltf::GLBHeader*)JSONData;
    if (glbHeader->magic == GLB_MAGIC)
    {
        // TODO: check
        std::unique_ptr<gltf::GLBInfo> glbInfo{ new gltf::GLBInfo };
        glbInfo->glbHeader = *glbHeader;
        
        glbInfo->jsonChunk = *(gltf::GLBChunk*)(JSONData + sizeof(gltf::GLBHeader));
        glbInfo->jsonChunk.chunkData = JSONData + sizeof(gltf::GLBHeader) + sizeof(gltf::GLBChunkHeader);
        
        if (glbHeader->length > sizeof(gltf::GLBHeader) + sizeof(gltf::GLBChunkHeader) + glbInfo->jsonChunk.chunkHeader.chunkLength)
        {
            glbInfo->binChunk = *(gltf::GLBChunk*)(glbInfo->jsonChunk.chunkData + glbInfo->jsonChunk.chunkHeader.chunkLength);
            glbInfo->binChunk.chunkData = glbInfo->jsonChunk.chunkData + glbInfo->jsonChunk.chunkHeader.chunkLength + sizeof(gltf::GLBChunkHeader);
        }
        
        JSONData = glbInfo->jsonChunk.chunkData;
        JSONLength = glbInfo->jsonChunk.chunkHeader.chunkLength;
        
        m_GLBInfo = std::move(glbInfo);
        m_isGLBFormat = true;
    }

    TIMER_BEG(t2);
    m_Doc.Parse((char*)JSONData, JSONLength);
    if (!m_Doc.IsObject())
    {
        LOG_ERROR("json parse error");
        return ERR_INVALID_ARG;
    }
    TIMER_END(t2, "glTF2_Loader: parse model");
    
    m_gltfResource = std::move(gltfBufferRes);

    TIMER_BEG(t3);
    //LoadExtensionsUsed();
    SceneComponentPtr rootSceneComponent = LoadDefaultScene();
    if (rootSceneComponent)
    {
        rootSceneComponent->SetName(filePath);
        scene = rootSceneComponent;

        animations.clear();
        LoadAnimation(animations);
    }
    TIMER_END(t3, "glTF2_Loader: load scene");

    return S_Success;
}

void glTF2_Loader::LoadExtensionsUsed()
{
    if (m_Doc.HasMember("extensionsUsed"))
    {
        Value& v = m_Doc["extensionsUsed"];
        if (v.IsArray())
        {
            m_ExtensionsUsed.resize(v.Size(), "");
            for (uint32_t i = 0; i < v.Size(); i++)
            {
                m_ExtensionsUsed[i] = v[i].GetString();
            }
        }
    }
}

SceneComponentPtr glTF2_Loader::LoadDefaultScene()
{
    SceneComponentPtr rootSC = nullptr;
    if (m_Doc.HasMember("scene"))
    {
        Value& v = m_Doc["scene"];
        uint32_t index = v.GetUint();
        rootSC = LoadScene(index);
    }
    return rootSC;
}

SceneComponentPtr glTF2_Loader::LoadScene(uint32_t index)
{
    SceneComponentPtr sc = nullptr;
    if (m_Doc.HasMember("scenes"))
    {
        Value& v_scenes = m_Doc["scenes"];
        if (v_scenes.IsArray() && index < v_scenes.Size())
        {
            sc = MakeSharedPtr<SceneComponent>(m_pContext);
            Value& v_scene = v_scenes[index];
            if (v_scene.HasMember("nodes"))
            {
                Value& v_nodes = v_scene["nodes"];
                if (v_nodes.IsArray())
                {
                    for (uint32_t i = 0; i < v_nodes.Size(); i++)
                    {
                        uint32_t node_idx = v_nodes[i].GetUint();
                        SceneComponentPtr child = this->LoadNode(node_idx);
                        if (child)
                        {
                            sc->AddChild(child);
                            child->SetParent(sc.get());
                        }
                    }
                }

                if (v_scene.HasMember("name") && sc)
                {
                    const char* name = v_scene["name"].GetString();
                    sc->SetName(name);
                }
            }
        }
    }
    return sc;
}

#define JsonValue2Float4(v) float4(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat(), v[3].GetFloat())
#define JsonValue2Float3(v) float3(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat())

SceneComponentPtr glTF2_Loader::LoadNode(uint32_t index)
{
    RETURN_IF_FOUND(m_Nodes, index);
    SceneComponentPtr sc = nullptr;
    if (m_Doc.HasMember("nodes"))
    {
        Value& v_nodes = m_Doc["nodes"];
        if (v_nodes.IsArray() && index < v_nodes.Size())
        {
            Value& v_node = v_nodes[index];
            if (v_node.HasMember("mesh"))
            {
                uint32_t mesh_index = v_node["mesh"].GetUint();
                if (v_node.HasMember("skin"))
                {
                    uint32_t skin_index = v_node["skin"].GetUint();
                    sc = this->LoadMesh(mesh_index, true, skin_index);
                }
                else
                {
                    sc = this->LoadMesh(mesh_index, false, 0);
                }
            }
            else
                sc = MakeSharedPtr<SceneComponent>(m_pContext);
            m_Nodes[index] = sc;

            for (auto member = v_node.MemberBegin(); member != v_node.MemberEnd(); member++)
            {
                const char* str = member->name.GetString();
                if (!strcmp(str, "children"))
                {
                    for (Value::ConstValueIterator it = member->value.Begin(); it != member->value.End(); ++it)
                    {
                        uint32_t node_index = it->GetUint();
                        SceneComponentPtr node = this->LoadNode(node_index);
                        if (node)
                        {
                            sc->AddChild(node);
                            node->SetParent(sc.get());
                        }
                    }
                }
                if (!strcmp(str, "matrix"))
                {
                    float mat[16];
                    float* p = mat;
                    Value& v_mat = member->value;
                    if (v_mat.IsArray() && v_mat.Size() == 16)
                    {
                        for (int32_t i = 0; i < 16; i++)
                        {
                            *p++ = v_mat[i].GetFloat();
                        }
                    }
                    Matrix4 mat4 = Matrix4(mat);
                    sc->SetLocalTransform(mat4);
                }
                if (!strcmp(str, "name"))
                {
                    sc->SetName(member->value.GetString());
                }
                if (!strcmp(str, "rotation"))
                {
                    Value& v_rot = member->value;
                    float4 rot = JsonValue2Float4(v_rot);
                    Quaternion qua = Quaternion(rot);
                    sc->SetLocalRotation(qua);
                }
                if (!strcmp(str, "scale"))
                {
                    Value& v_scale = member->value;
                    float3 scale = JsonValue2Float3(v_scale);
                    sc->SetLocalScale(scale);
                }
                if (!strcmp(str, "translation"))
                {
                    Value& v_trans = member->value;
                    float3 pos = JsonValue2Float3(v_trans);
                    sc->SetLocalTranslation(pos);
                }

                //if (!strcmp(str, "extensions"))
                //{
                //    Value& v_ext = member->value;
                //    for (auto member_ext = v_ext.MemberBegin(); member_ext != v_ext.MemberEnd(); member_ext++)
                //    {
                //        const char* str_ext = member_ext->name.GetString();
                //        if (!strcmp(str_ext, "KHR_lights_punctual"))
                //        {
                //            Value& v_ext_lights_punctual = member_ext->value;
                //            if (v_ext_lights_punctual.HasMember("light"))
                //            {
                //                Value& v_light = v_ext_lights_punctual["light"];
                //                uint32_t light_index = v_light.GetUint();
                //                LightComponentPtr light = this->LoadLight(light_index);
                //                if (light)
                //                {
                //                    sc->AddChild(light);
                //                    light->SetParent(sc.get());
                //                }
                //            }
                //        }
                //    }
                //}
            }

            if(sc->GetName() != "")
                m_NodesMapByName[sc->GetName()] = sc;
        }
    }

    return sc;
}

MeshComponentPtr glTF2_Loader::LoadMesh(uint32_t mesh_index, bool has_skin, uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Meshes, mesh_index);
    MeshComponentPtr meshComponent = nullptr;
    if (m_Doc.HasMember("meshes"))
    {
        Value& v_meshes = m_Doc["meshes"];
        if (v_meshes.IsArray() && mesh_index < v_meshes.Size())
        {
            Value& v_mesh = v_meshes[mesh_index];

            LoaderSkinPtr skin = nullptr;
            if (has_skin)
            {
                skin = LoadSkin(skin_index);
                SkeletalMeshComponentPtr sklMeshComponent = MakeSharedPtr<SkeletalMeshComponent>(m_pContext);
                sklMeshComponent->SetInverseBindMatrix(skin->inverseBindMatrices);
                sklMeshComponent->SetJoint(skin->joints);
                sklMeshComponent->SetSkeletonRoot(skin->skeleton);
                meshComponent = sklMeshComponent;
            }
            else
                meshComponent = MakeSharedPtr<MeshComponent>(m_pContext);
            m_Meshes[mesh_index] = meshComponent;

            string name;
            if (v_mesh.HasMember("name"))
            {
                name = v_mesh["name"].GetString();
                meshComponent->SetName(name);
            }

            vector<float> weights;
            if (v_mesh.HasMember("weights") && v_mesh["weights"].IsArray())
            {
                Value& v_weights = v_mesh["weights"];
                weights.resize(v_weights.Size());
                for (uint32_t i = 0; i < v_weights.Size(); i++)
                {
                    weights[i] = v_weights[i].GetFloat();
                }
            }

            vector<string> targetNames;
            if (v_mesh.HasMember("extras"))
            {
                Value& v_extras = v_mesh["extras"];
                if (v_extras.HasMember("targetNames") && v_extras["targetNames"].IsArray())
                {
                    Value& v_targetNames = v_extras["targetNames"];
                    if (weights.size() != v_targetNames.Size())
                    {
                        LOG_ERROR("Morph target name size error");
                        return meshComponent;
                    }
                    targetNames.resize(v_targetNames.Size(), "");
                    for (uint32_t i = 0; i < v_targetNames.Size(); i++)
                    {
                        string targetName = v_targetNames[i].GetString();
                        //targetNames.push_back(targetName);
                        targetNames[i] = targetName;
                    }
                }
            }

            if (v_mesh.HasMember("primitives") && v_mesh["primitives"].IsArray())
            {
                Value& v_primitives = v_mesh["primitives"];
                AABBox meshes_box;
                meshes_box.Min(float3(0.0f));
                meshes_box.Max(float3(0.0f));
                for (uint32_t i = 0; i < v_primitives.Size(); i++)
                {
                    RHIMeshPtr mesh = LoadPrimitive(v_primitives[i]);
                    if (mesh)
                    {
                        meshComponent->AddMesh(mesh);
                        MorphInfo& morphTarget = mesh->GetMorphTargetResource()._morphInfo;
                        if (morphTarget.morph_target_names.size() == 0 && targetNames.size() != 0)
                        {
                            morphTarget.morph_target_names = targetNames;
                        }
                        if (weights.size() != 0)
                            morphTarget.morph_target_weights = weights;

                        AABBox mesh_box = mesh->GetAABBox();
                        if (i == 0)
                            meshes_box = mesh_box;
                        else
                            meshes_box |= mesh_box;
                    }
                }
                meshComponent->SetAABBox(meshes_box);
            }
        }
    }
    return meshComponent;
}

LightComponentPtr glTF2_Loader::LoadLight(uint32_t light_index)
{
    RETURN_IF_FOUND(m_Lights, light_index);
    LightComponentPtr lightComponent = nullptr;
    if (m_Doc.HasMember("extensions"))
    {
        Value& v_ext = m_Doc["extensions"];
        if (v_ext.HasMember("KHR_lights_punctual"))
        {
            Value& v_ext_lights_punctual = v_ext["KHR_lights_punctual"];
            if (v_ext_lights_punctual.HasMember("lights"))
            {
                Value& v_lights = v_ext_lights_punctual["lights"];
                if (v_lights.IsArray() && light_index < v_lights.Size())
                {
                    Value& v_light = v_lights[light_index];
                    if (v_light.HasMember("type"))
                    {
                        string type = v_light["type"].GetString();
                        if (type == "directional")
                            lightComponent = MakeSharedPtr<DirectionalLightComponent>(m_pContext);
                        else if (type == "point")
                            lightComponent = MakeSharedPtr<PointLightComponent>(m_pContext);
                        else if (type == "spot")
                            lightComponent = MakeSharedPtr<SpotLightComponent>(m_pContext);
                        else
                        {
                            LOG_ERROR("Light type error\n");
                            return lightComponent;
                        }
                    }
                    m_Lights[light_index] = lightComponent;

                    if (v_light.HasMember("name"))
                    {
                        string name = v_light["name"].GetString();
                        lightComponent->SetName(name);
                    }
                    if (v_light.HasMember("color"))
                    {
                        float3 c = JsonValue2Float3(v_light["color"]);
                        lightComponent->SetColor(Color(uint8_t(c[0] * 255.0f), uint8_t(c[1] * 255.0f), uint8_t(c[2] * 255.0f)));
                    }
                    if (v_light.HasMember("intensity"))
                    {
                        lightComponent->SetIntensity(v_light["intensity"].GetFloat());
                    }
                    //if (v_light.HasMember("range"))
                    //{
                    //}
                }
            }
        }
    }
    return lightComponent;
}

RHIMeshPtr glTF2_Loader::LoadPrimitive(Value& v_primitive)
{
    RHIMeshPtr mesh = nullptr;
    if (v_primitive.HasMember("attributes"))
    {
        VertexAttributeResource vertexAttributeRes;
        
        RHIContext* rc = &m_pContext->RHIContextInstance();
        mesh = rc->CreateMesh();
        if (!mesh)
            return mesh;

        if (v_primitive.HasMember("mode"))
        {
            int mode = v_primitive["mode"].GetInt();
            mesh->SetTopologyType(gltf::ConvertToTopologyType(mode));
            
        }

        Value& v_attr = v_primitive["attributes"];
        bool hasPosition = false;
        bool hasNormal = false;
        bool hasTexcoord = false;
        bool hasTangent = false;
        SkinningJointBindSize joint_bind_size = SkinningJointBindSize::None;
        std::map<uint32_t, VertexStream> vertexStreams;
        std::map<uint32_t, std::shared_ptr<BufferResource>> blendIndexBufferRes;
        for (Value::ConstMemberIterator itr = v_attr.MemberBegin(); itr != v_attr.MemberEnd(); ++itr)
        {
            auto vertexAttribute = gltf::ConvertVertexAttribute(itr->name.GetString());
            uint32_t accessor_index = itr->value.GetUint();
            
            LoaderAccessorPtr accessor = LoadAccessor(accessor_index);
            LoaderBufferViewPtr bv = LoadBufferView(accessor->bufferViewIndex);
            
            VertexStream* vertexStream;
            // now always create a new vertex stream
            //if (vertexStreams.find(accessor->bufferViewIndex) != vertexStreams.end())
            //{
            //    vertexStream = &vertexStreams[accessor->bufferViewIndex];
            //}
            //else
            {
                vertexStreams.emplace(accessor->bufferViewIndex, VertexStream{});
                vertexStream = &vertexStreams[accessor->bufferViewIndex];
                if (bv->byteStride != GLTF_INVALID_INTEGER)
                    vertexStream->stride = bv->byteStride;
                else
                    vertexStream->stride = gltf::ElementSize(accessor->componentType, accessor->elementType);
                vertexStream->offset = 0; //bv->byteOffset;
            }
            
            VertexStreamLayout layout;
            layout.buffer_offset = accessor->byteOffset;
            layout.format = gltf::ConvertToVertexFormat(accessor->elementType, accessor->componentType);;
            layout.usage = vertexAttribute.first;
            layout.usage_index = vertexAttribute.second;

            // gltf only support unsigned byte or unsigned short
            // but engine always use unsigned integer, so need to convert to UInt4
            // TODO: maybe hardcode here is a bad idea. put it in application level?
            if (vertexAttribute.first == VertexElementUsage::BlendIndex /*&& layout.format == VertexFormat::Unknown*/)
            {
                layout.format = VertexFormat::UInt4;
                uint32_t jointsDataSize = accessor->count * 4;
                std::shared_ptr<uint32_t> _backJointData{ new uint32_t[jointsDataSize], default_array_deleter<uint32_t>() };
                std::shared_ptr<BufferResource> _backJointBuf = MakeSharedPtr<BufferResource>();
                _backJointBuf->_uninitializer = [_backJointData](IResource*) mutable {
                    _backJointData.reset();
                };
                _backJointBuf->_data = (uint8_t*)_backJointData.get();
                _backJointBuf->_size = jointsDataSize * sizeof(uint32_t);

                if (accessor->componentType == GLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                {
                    uint8_t* data = (uint8_t*)(bv->binaryData + accessor->byteOffset);
                    for (int i = 0; i < jointsDataSize; i++)
                        _backJointData.get()[i] = (uint32_t)data[i];
                }
                else if (accessor->componentType == GLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    uint16_t* data = (uint16_t*)(bv->binaryData + accessor->byteOffset);
                    for (int i = 0; i < jointsDataSize; i++)
                        _backJointData.get()[i] = data[i];
                }
                vertexStream->stride = 16;
                blendIndexBufferRes.emplace(accessor->bufferViewIndex, std::move(_backJointBuf));
            }
            vertexStream->layouts.push_back(layout);

            if (vertexAttribute.first == VertexElementUsage::Position)
            {
                hasPosition = true;
                AABBox mesh_box;
                
                mesh_box.Max(float3{ accessor->maxValues.data() });
                mesh_box.Min(float3{ accessor->minValues.data() });
                mesh->SetAABBox(mesh_box);

#if 0
                LoaderBufferViewPtr bv = LoadBufferView(accessor->bufferViewIndex);
                float* data = (float*)((char*)(bv->buffer->data.data()) + accessor->byteOffset + bv->byteOffset);
                float xmax = data[0];
                float ymax = data[1];
                float zmax = data[2];
                float xmin = data[0];
                float ymin = data[1];
                float zmin = data[2];
                for (int32_t i = 1; i < accessor->count; i++)
                {
                    xmax = max(data[i * 3], xmax);
                    ymax = max(data[i * 3 + 1], ymax);
                    zmax = max(data[i * 3 + 2], zmax);
                    xmin = min(data[i * 3], xmin);
                    ymin = min(data[i * 3 + 1], ymin);
                    zmin = min(data[i * 3 + 2], zmin);
                }
                //LOG_INFO("xmax = %f, ymax = %f, zmax = %f\n", xmax, ymax, zmax);
                //LOG_INFO("xmin = %f, ymin = %f, zmin = %f\n", xmin, ymin, zmin);
                if (xmax != mesh_box.Max()[0] && ymax != mesh_box.Max()[1] &&
                    zmax != mesh_box.Max()[2] && xmin != mesh_box.Min()[0] &&
                    ymin != mesh_box.Min()[1] && zmin != mesh_box.Min()[2])
                {
                    LOG_INFO("not match\n");
                }
#endif
            }
            else if (vertexAttribute.first == VertexElementUsage::Normal)
                hasNormal = true;
            else if (vertexAttribute.first == VertexElementUsage::TexCoord)
                hasTexcoord = true;
            else if(vertexAttribute.first == VertexElementUsage::BlendWeight)
                joint_bind_size = (SkinningJointBindSize)((vertexAttribute.second + 1) * 4);
            else if (vertexAttribute.first == VertexElementUsage::Tangent)
                hasTangent = true;
        }
        
        for (auto& _vertexStreamPair : vertexStreams)
        {
            vertexAttributeRes._vertexStreams.push_back(_vertexStreamPair.second);
            if (blendIndexBufferRes.find(_vertexStreamPair.first) != blendIndexBufferRes.end())
                vertexAttributeRes._vertexBuffers.push_back(blendIndexBufferRes[_vertexStreamPair.first]);
            else
                vertexAttributeRes._vertexBuffers.push_back(m_BufferViews[_vertexStreamPair.first]->backendBuffer);
        }
        mesh->SetVertexAttributeResource(vertexAttributeRes);
        
        mesh->SetSkinningJointBindSize(joint_bind_size);
        if(!hasNormal)
            LOG_ERROR("Lack normal in the mesh\n");
        if (!hasTexcoord)
            LOG_ERROR("Lack texcoord in the mesh\n");

        if(v_primitive.HasMember("indices"))
        {
            uint32_t accessor_index = v_primitive["indices"].GetUint();
            LoaderAccessorPtr accessor = LoadAccessor(accessor_index);
            LoaderBufferViewPtr bv = LoadBufferView(accessor->bufferViewIndex);
            
            std::shared_ptr<VertexIndicesResource> indicesResource = MakeSharedPtr<VertexIndicesResource>();
            indicesResource->_indexBufferType = gltf::ConvertToIndexBufferType(accessor->componentType);
            indicesResource->_indexCount = accessor->count;
            indicesResource->_data = bv->binaryData + accessor->byteOffset;
            indicesResource->_size = accessor->count * gltf::ComponentByteSize(accessor->componentType);
            mesh->SetIndexBufferResource(indicesResource);
        }

        if (v_primitive.HasMember("targets") && v_primitive["targets"].IsArray())
        {
            AABBox targets_box;
            targets_box.Min(float3(0.0f));
            targets_box.Max(float3(0.0f));
            this->ConverterToMorphStreamUnitFromTargets(mesh, v_primitive["targets"], targets_box);
            //temporary solution
            AABBox mesh_box = mesh->GetAABBox();
            mesh->SetAABBox(mesh_box + targets_box);
        }

        if (v_primitive.HasMember("material"))
        {
            uint32_t material_index = v_primitive["material"].GetUint();
            std::shared_ptr<MaterialResource> material = LoadMaterial(material_index, hasTangent);
            if (material)
                mesh->SetMaterialResource(material);
            else
            {
                std::shared_ptr<MaterialResource> material = MakeSharedPtr<MaterialResource>();
                mesh->SetMaterialResource(material);
            }
        }
        else
        {
            MaterialPtr material = MakeSharedPtr<Material>();
            mesh->SetMaterial(material);
        }
    }

    return mesh;
}

std::shared_ptr<MaterialResource> glTF2_Loader::LoadMaterial(uint32_t material_index, bool hasTangent)
{
    RETURN_IF_FOUND(m_Materials, material_index);
    std::shared_ptr<MaterialResource> material = nullptr;
    if (m_Doc.HasMember("materials"))
    {
        Value& v_materials = m_Doc["materials"];
        if (v_materials.IsArray() && material_index < v_materials.Size())
        {
            material = MakeSharedPtr<MaterialResource>();
            Value& v = v_materials[material_index];
            for (auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
            {
                const char* name = it->name.GetString();
                if (!strcmp(name, "pbrMetallicRoughness"))
                {
                    Value& v_metal_rough = it->value;
                    this->LoadMetallicRoughness(v_metal_rough, *material.get());
                }
                else if (!strcmp(name, "name"))
                {
                    Value& v_name = it->value;
                    material->_name = v_name.GetString();
                }
                else if (!strcmp(name, "normalTexture"))
                {
                    if (hasTangent)
                    {
                        Value& v_normal_info = it->value;
                        if (v_normal_info.HasMember("index"))
                        {
                            Value& v_index = v_normal_info["index"];
                            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_normal_info);
                            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
                            material->_normalImage = img->bitmapData;
                        }
                    }
                    else
                        LOG_WARNING("Has normal texture, but lack tangent data in the mesh\n");
                }
                else if (!strcmp(name, "occlusionTexture"))
                {
                    Value& v_occlusion_info = it->value;
                    if (v_occlusion_info.HasMember("index"))
                    {
                        Value& v_index = v_occlusion_info["index"];
                        LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_occlusion_info);
                        LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
                        material->_occlusionImage = img->bitmapData;
                    }
                }
                else if (!strcmp(name, "emissiveTexture"))
                {
                    Value& v_emissive = it->value;
                    if (v_emissive.HasMember("index"))
                    {
                        Value& v_index = v_emissive["index"];
                        int32_t tex_index = v_index.GetUint();
                        LoaderTexturePtr tex = this->LoadTexture(tex_index, v_emissive);
                        LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
                        material->_emmissiveImage = img->bitmapData;
                    }
                }
                else if (!strcmp(name, "emissiveFactor"))
                {
                    Value& v_emi_factor = it->value;
                    material->_emissiveFactor = JsonValue2Float3(v_emi_factor);
                }
                else if (!strcmp(name, "alphaMode"))
                {
                    AlphaMode am = AlphaMode::Opaque;
                    const char* mode = it->value.GetString();
                    if (!strcmp(mode, "MASK"))
                        am = AlphaMode::Mask;
                    else if (!strcmp(mode, "BLEND"))
                        am = AlphaMode::Blend;
                    material->_alphaMode = am;
                }
                else if (!strcmp(name, "alphaCutoff"))
                {
                    float cutoff = it->value.GetFloat();
                    material->_alphaCutoff = cutoff;
                }
                else if (!strcmp(name, "doubleSided"))
                {
                    bool b = it->value.GetBool();
                    material->_doubleSided = b;
                }

                //extensions
                if (!strcmp(name, "extensions"))
                {
                    Value& v_ext = it->value;
                    for (auto it_ext = v_ext.MemberBegin(); it_ext != v_ext.MemberEnd(); it_ext++)
                    {
                        const char* name_ext = it_ext->name.GetString();
                        if (!strcmp(name_ext, "KHR_materials_clearcoat"))
                        {
                            Value& v_ext_clearcoat = it_ext->value;
                            this->LoadClearcoat(v_ext_clearcoat, *material.get());
                        }
                        else if (!strcmp(name_ext, "KHR_materials_sheen"))
                        {
                            Value& v_ext_sheen = it_ext->value;
                            this->LoadSheen(v_ext_sheen, *material.get());
                        }
                        else if (!strcmp(name_ext, "KHR_materials_ior"))
                        {
                            material->_IORFactor = it_ext->value.GetFloat();
                        }
                    }
                }
            }
            m_Materials[material_index] = material;
        }
    }
    return material;
}

bool glTF2_Loader::LoadMetallicRoughness(Value& v, MaterialResource& pMaterial)
{
    if (v.HasMember("baseColorFactor"))
    {
        Value& v_bcf = v["baseColorFactor"];
        pMaterial._albedoFactor = JsonValue2Float4(v_bcf);
    }
    if (v.HasMember("baseColorTexture"))
    {
        Value& v_bct = v["baseColorTexture"];
        if (v_bct.HasMember("index"))
        {
            Value& v_index = v_bct["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_bct);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, true);
            pMaterial._albedoImage = img->bitmapData;
            //img->bitmapData->DumpToFile("D:/aaa.rgb");
        }
    }
    if (v.HasMember("metallicFactor"))
    {
        Value& v_mf = v["metallicFactor"];
        pMaterial._metallicFactor = v_mf.GetFloat();
    }
    if (v.HasMember("roughnessFactor"))
    {
        Value& v_rf = v["roughnessFactor"];
        pMaterial._roughnessFactor = v_rf.GetFloat();
    }
    if (v.HasMember("metallicRoughnessTexture"))
    {
        Value& v_mrt = v["metallicRoughnessTexture"];
        if (v_mrt.HasMember("index"))
        {
            Value& v_index = v_mrt["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_mrt);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
            pMaterial._metallicRoughnessImage = img->bitmapData;
        }
    }
    return true;
}

bool glTF2_Loader::LoadClearcoat(rapidjson::Value& v, MaterialResource& pMaterial)
{
    if (v.HasMember("clearcoatFactor"))
    {
        Value& v_cf = v["clearcoatFactor"];
        pMaterial._clearcoatFactor = v_cf.GetFloat();
    }
    if (v.HasMember("clearcoatRoughnessFactor"))
    {
        Value& v_crf = v["clearcoatRoughnessFactor"];
        pMaterial._clearcoatRoughnessFactor = v_crf.GetFloat();
    }
    if (v.HasMember("clearcoatTexture"))
    {
        Value& v_ct = v["clearcoatTexture"];
        if (v_ct.HasMember("index"))
        {
            Value& v_index = v_ct["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_ct);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
            pMaterial._clearcoatImage = img->bitmapData;
        }
    }
    if (v.HasMember("clearcoatRoughnessTexture"))
    {
        Value& v_crt = v["clearcoatRoughnessTexture"];
        if (v_crt.HasMember("index"))
        {
            Value& v_index = v_crt["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_crt);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
            pMaterial._clearcoatRoughnessImage = img->bitmapData;
        }
    }
    return true;
}

bool glTF2_Loader::LoadSheen(rapidjson::Value& v, MaterialResource& pMaterial)
{
    if (v.HasMember("sheenColorFactor"))
    {
        Value& v_scf = v["sheenColorFactor"];
        pMaterial._sheenColorFactor = (JsonValue2Float3(v_scf));
    }
    if (v.HasMember("sheenRoughnessFactor"))
    {
        Value& v_srf = v["sheenRoughnessFactor"];
        pMaterial._sheenRoughnessFactor = (v_srf.GetFloat());
    }
    if (v.HasMember("sheenColorTexture"))
    {
        Value& v_sct = v["sheenColorTexture"];
        if (v_sct.HasMember("index"))
        {
            Value& v_index = v_sct["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_sct);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
            pMaterial._sheenColorImage = (img->bitmapData);
        }
    }
    if (v.HasMember("sheenRoughnessTexture"))
    {
        Value& v_srt = v["sheenRoughnessTexture"];
        if (v_srt.HasMember("index"))
        {
            Value& v_index = v_srt["index"];
            LoaderTexturePtr tex = this->LoadTexture(v_index.GetUint(), v_srt);
            LoaderImagePtr img = this->LoadImage(tex->imageIndex, false);
            pMaterial._sheenRoughnessImage = (img->bitmapData);
        }
    }
    return true;
}

LoaderTexturePtr glTF2_Loader::LoadTexture(uint32_t tex_index, Value& v_mat)
{
    RETURN_IF_FOUND(m_Textures, tex_index);
    LoaderTexturePtr tex = nullptr;
    if (m_Doc.HasMember("textures"))
    {
        Value& v_texs = m_Doc["textures"];
        if (v_texs.IsArray() && tex_index < v_texs.Size())
        {
            tex = MakeSharedPtr<LoaderTexture>();
            Value& v_tex_index = v_texs[tex_index];
            tex->imageIndex = v_tex_index["source"].GetUint();
            //tex->samplerIndex = v_tex_index["sampler"].GetUint();

            if (v_mat.HasMember("texCoord"))
            {
                Value& v_texCoord = v_mat["texCoord"];
                tex->texCoord = v_texCoord.GetUint();
            }

            if (v_mat.HasMember("scale"))
            {
                Value& v_scale = v_mat["scale"];
                tex->scale = v_scale.GetFloat();
            }

            m_Textures[tex_index] = tex;
        }
    }
    return tex;
}

LoaderImagePtr glTF2_Loader::LoadImage(uint32_t image_index, bool bColor)
{
    RETURN_IF_FOUND(m_Images, image_index);
    LoaderImagePtr img = nullptr;
    if (m_Doc.HasMember("images"))
    {
        Value& v_images = m_Doc["images"];
        if (v_images.IsArray() && image_index < v_images.Size())
        {
            img = MakeSharedPtr<LoaderImage>();
            Value& v = v_images[image_index];
            ImageType imageType = ImageType::UNKNOWN;
            string mimeType;
            if (v.HasMember("mimeType"))
            {
                mimeType = v["mimeType"].GetString();
                img->mimeType = mimeType;
                if (mimeType == "image/jpeg")
                {
                    imageType = ImageType::JPEG;
                }
                else if (mimeType == "image/png")
                {
                    imageType = ImageType::PNG;
                }
                else
                {
                    LOG_ERROR("error\n");
                    return nullptr;
                }
            }

            if (v.HasMember("bufferView"))
            {                
                int32_t bvIndex = v["bufferView"].GetUint();
                
                img->bufferViewIndex = bvIndex;
                img->bitmapData = nullptr;
                for (auto it: m_Images)
                {
                    if (it.second->bufferViewIndex == bvIndex && it.second->mimeType == mimeType)
                    {
                        img->bitmapData = it.second->bitmapData;
                        LOG_INFO("Find a texture2D");
                        break;
                    }
                }

                if (!img->bitmapData)
                {
                    LoaderBufferViewPtr bv = LoadBufferView(bvIndex);
                    if (bv)
                    {
                        uint8_t* data = (uint8_t*)(bv->binaryData);
                        BitmapBufferPtr bm = ImageDecode(data, uint32_t(bv->byteLength), imageType);
                        //FILE* f = fopen("a.yuv", "wb");
                        //fwrite(bm->Data(), 1, 1024 * 1024 * 4, f);
                        //fclose(f);
//                        RenderContext* rc = &m_pContext->RenderContextInstance();
//                        Texture::Desc tex_desc;
//                        tex_desc.type = TextureType::Tex2D;
//                        tex_desc.width = bm->Width();
//                        tex_desc.height = bm->Height();
//                        if (!bColor)
//                        {
//                            tex_desc.format = bm->Format();
//                        }
//                        else
//                        {
//                            if (bm->Format() == PixelFormat::R8G8B8A8_UNORM)
//                            {
//                                if (bm->GetColorSpace() == ColorSpace::Unknown || bm->GetColorSpace() == ColorSpace::sRGB)
//                                {
//                                    tex_desc.format = PixelFormat::R8G8B8A8_UNORM_SRGB;
//                                }
//                                else
//                                {
//                                    LOG_ERROR("Unsupported color space %d", (int)ColorSpace::sRGB);
//                                    tex_desc.format = bm->Format();
//                                }
//                            }
//                            else
//                            {
//                                tex_desc.format = bm->Format();
//                            }
//                        }
//                        tex_desc.flags = RESOURCE_FLAG_SRV;
//                        TexturePtr texPtr = rc->CreateTexture2D(tex_desc, bm);

                        //img->texture2DPtr = texPtr;
                        img->bitmapData = bm;
                        LOG_INFO("Create a texture2D");
                    }
                }
            }
            else if (v.HasMember("uri"))
            {
                img->uriPath = v["uri"].GetString();

                std::string uriPath = m_filePath.substr(0, m_filePath.find_last_of('/')) + "/" + img->uriPath;
                std::shared_ptr<FileResource> uriFileRes = MakeSharedPtr<FileResource>(uriPath);
                if (!uriFileRes->IsAvailable())
                {
                    LOG_ERROR("gltf_Loader: load uri file %s fail", uriPath.c_str());
                    return nullptr;
                }
                
                uint8_t* data = (uint8_t*)(uriFileRes->_data);
                BitmapBufferPtr bm = ImageDecode(data, uint32_t(uriFileRes->_size), imageType);
                img->bitmapData = bm;
            }
            //if (v.HasMember("name"))
            //    image->Name(v[NAME].GetString());

            m_Images[image_index] = img;
        }
    }
    return img;
}

LoaderSkinPtr glTF2_Loader::LoadSkin(uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Skins, skin_index);
    LoaderSkinPtr skin = nullptr;
    if (m_Doc.HasMember("skins"))
    {
        Value& v_skins = m_Doc["skins"];
        if (v_skins.IsArray() && skin_index < v_skins.Size())
        {
            skin = MakeSharedPtr<LoaderSkin>();
            Value& v = v_skins[skin_index];
            if (v.HasMember("inverseBindMatrices"))
            {
                uint32_t access_index = v["inverseBindMatrices"].GetUint();
                LoaderAccessorPtr accessor = LoadAccessor(access_index);
                if (accessor)
                {
                    if (accessor->elementType != GLTF_ELEMENT_TYPE_MAT4)
                        return nullptr;
                    LoaderBufferViewPtr bv = LoadBufferView(accessor->bufferViewIndex);
                    if (bv)
                    {
                        float* data = (float*)((uint8_t*)(bv->binaryData) + accessor->byteOffset);
                        for (size_t i = 0; i < accessor->count; i++)
                        {
                            skin->inverseBindMatrices.push_back(Matrix4(data));
                            data += 16;
                        }
                    }
                }
            }
            if (v.HasMember("joints"))
            {
                Value& v_joints = v["joints"];
                size_t count = v_joints.Size();
                if(count != skin->inverseBindMatrices.size())
                    LOG_ERROR("skin joints size is not correct");
                for (size_t i = 0; i < count; i++)
                {
                    uint32_t joint_index = (uint32_t)v_joints[(rapidjson::SizeType)i].GetUint();
                    SceneComponentPtr sc = this->LoadNode(joint_index);
                    if (sc)
                        skin->joints.push_back(sc);
                    else
                        LOG_ERROR("skin joints index is not correct");
                }
            }
            if (v.HasMember("skeleton"))
            {
                uint32_t joint_index = v["skeleton"].GetUint();
                SceneComponentPtr skeleton_sc = this->LoadNode(joint_index);
                if (skeleton_sc)
                    skin->skeleton = skeleton_sc;
                else
                    LOG_ERROR("skin skeleton index is not correct");
            }
            //if (v.HasMember("name"))
            //    skin->Name(v["name"].GetString());

            m_Skins[skin_index] = skin;
        }
    }
    return skin;
}

void glTF2_Loader::LoadAnimation(vector<AnimationComponentPtr>& animations)
{
    float start_time = 0.0;
    float end_time = 0.0;
    if (m_Doc.HasMember("animations"))
    {
        Value& v_anims = m_Doc["animations"];
        int32_t anim_count = v_anims.Size();
        for (int32_t anim_index = 0; anim_index < anim_count; anim_index++)
        {
            Value& v = v_anims[anim_index];
            AnimationComponentPtr anim = MakeSharedPtr<AnimationComponent>(m_pContext);
            if (v.HasMember("name"))
            {
                std::string name = v["name"].GetString();
                anim->SetName(name);
            }

            if (v.HasMember("channels") && v["channels"].IsArray() && v["channels"].Size() > 0 &&
                v.HasMember("samplers") && v["samplers"].IsArray() && v["samplers"].Size() > 0)
            {
                Value& v_channels = v["channels"];
                int32_t channel_count = v_channels.Size();
                m_AnimSamplers.clear();
                for (int32_t i = 0; i < channel_count; i++)
                {
                    int32_t sampler_index = 0;
                    int32_t target_node_index = 0;
                    std::string target_path;
                    Value& v_channel = v_channels[i];
                    if (v_channel.HasMember("sampler"))
                        sampler_index = v_channel["sampler"].GetUint();
                    if (v_channel.HasMember("target"))
                    {
                        Value& v_target = v_channel["target"];
                        if (v_target.HasMember("node"))
                            target_node_index = v_target["node"].GetUint();
                        if (v_target.HasMember("path"))
                            target_path = v_target["path"].GetString();
                    }
                    TransformType type = TransformType::None;
                    if (target_path == "translation")   type = Translate;
                    else if (target_path == "rotation") type = Rotate;
                    else if (target_path == "scale")    type = Scale;
                    SceneComponentPtr sc = LoadNode(target_node_index);
                    if (sc == nullptr)
                    {
                        LOG_ERROR("LoadAnimation Error");
                        continue;
                    }

                    AnimationTrackPtr animTrack = nullptr;
                    TransformAnimationTrackPtr tAnimTrack = nullptr;
                    MorphTargetAnimationTrackPtr mtAnimTrack = nullptr;
                    if (type == Translate || type == Rotate || type == Scale)
                    {
                        tAnimTrack = anim->CreateTransformTrack();
                        tAnimTrack->SetSceneComponent(sc);
                        animTrack = tAnimTrack;
                    }
                    else if(target_path == "weights")
                    {
                        mtAnimTrack = anim->CreateMorphTargetTrack();
                        mtAnimTrack->SetSceneComponent(sc);
                        animTrack = mtAnimTrack;
                    }
                    else
                    {
                        LOG_ERROR("LoadAnimation Error");
                        continue;
                    }

                    LoaderAnimSamplerPtr animSampler = LoadAnimationSampler(sampler_index, v);
                    if (!animSampler && (animSampler->inputDataCount != animSampler->outputDataCount))
                        LOG_ERROR("Animation sampler data count error.");
                    animTrack->SetInterpolationType(animSampler->ipType);

                    uint32_t count = animSampler->inputDataCount;
                    for (uint32_t i = 0; i < count; i++)
                    {
                        if (nullptr != tAnimTrack)
                        {
                            TransformKeyFramePtr tKFPtr = nullptr;
                            tKFPtr = tAnimTrack->CreateTransformKeyFrame();
                            float time = animSampler->inputData[i];
                            tKFPtr->SetTime(time);
                            start_time = i == 0 ? time : start_time;
                            end_time = time;
                            if (type == Translate && animSampler->outputDataStride == 12)
                            {
                                float* src = ((float*)(animSampler->outputData)) + 3 * i;
                                float3 translation = float3(*src, *(src + 1), *(src + 2));
                                tKFPtr->SetTranslate(translation);
                                tKFPtr->SetOffsetTranslate(translation - sc->GetLocalTransform().GetTranslation());
                            }
                            else if (type == Rotate && animSampler->outputDataStride == 16)
                            {
                                float* src = ((float*)(animSampler->outputData)) + 4 * i;
                                Quaternion qua = Quaternion(*src, *(src + 1), *(src + 2), *(src + 3));
                                tKFPtr->SetRotate(qua);
                                tKFPtr->SetOffsetRotate((sc->GetLocalTransform().GetRotation().Inverse()) * qua);
                            }
                            else if (type == Scale && animSampler->outputDataStride == 12)
                            {
                                float* src = ((float*)(animSampler->outputData)) + 3 * i;
                                float3 scale = float3(*src, *(src + 1), *(src + 2));
                                tKFPtr->SetScale(scale);
                                tKFPtr->SetOffsetScale(scale / sc->GetLocalTransform().GetScale());
                            }
                            else
                                LOG_ERROR("not support");
                        }
                        else if (nullptr != mtAnimTrack)
                        {
                            MorphTargetKeyFramePtr mtKFPtr = mtAnimTrack->CreateMorphTargetKeyFrame();
                            float time = animSampler->inputData[i];
                            mtKFPtr->SetTime(time);
                            start_time = i == 0 ? time : start_time;
                            end_time = time;

                            if (animSampler->outputDataStride == 4)
                            {
                                uint32_t weight_count = animSampler->outputDataCount / count;
                                float* weight_ptr = (float*)(animSampler->outputData) + i * weight_count;
                                std::vector<float> weights(weight_count);
                                for (uint32_t k = 0; k < weight_count; k++)
                                    weights[k] = weight_ptr[k];
                                mtKFPtr->SetWeights(weights);
                            }

                        }
                    }
                }
            }

            if (anim)
            {
                AnimInfo animInfo;
                animInfo.startTime = start_time;
                animInfo.preFrameTime = start_time;
                animInfo.endTime = end_time;
                animInfo.state = AnimationState::Stopped;
                animInfo.loop = true;
                anim->AddAnimSectionInfo(animInfo);
            }

            animations.push_back(anim);
        }
    }

    return;
}

LoaderAnimSamplerPtr glTF2_Loader::LoadAnimationSampler(uint32_t sampler_index, rapidjson::Value& v_anim)
{
    RETURN_IF_FOUND(m_AnimSamplers, sampler_index);
    LoaderAnimSamplerPtr sampler = MakeSharedPtr<LoaderAnimSampler>();
    Value& v_sampler = v_anim["samplers"][sampler_index];
    uint32_t input_acc_idx = 0;
    uint32_t output_acc_idx = 0;
    if (v_sampler.HasMember("input"))
        input_acc_idx = v_sampler["input"].GetUint();
    if (v_sampler.HasMember("output"))
        output_acc_idx = v_sampler["output"].GetUint();
    if (v_sampler.HasMember("interpolation"))
        sampler->ipType = gltf::ConvertToInterpolationType(v_sampler["interpolation"].GetString());

    LoaderAccessorPtr input_accessor = LoadAccessor(input_acc_idx);
    if (input_accessor && input_accessor->componentType == GLTF_COMPONENT_TYPE_FLOAT && input_accessor->elementType == GLTF_ELEMENT_TYPE_SCALAR)
    {
        LoaderBufferViewPtr bv = LoadBufferView(input_accessor->bufferViewIndex);
        if (!bv)
            return nullptr;
        sampler->inputData = (float*)(bv->binaryData + input_accessor->byteOffset);
        sampler->inputDataCount = (int32_t)input_accessor->count;
    }

    LoaderAccessorPtr output_accessor = this->LoadAccessor(output_acc_idx);
    if (output_accessor && input_accessor->componentType == GLTF_COMPONENT_TYPE_FLOAT)
    {
        LoaderBufferViewPtr bv = LoadBufferView(output_accessor->bufferViewIndex);
        if (!bv)
            return nullptr;
        sampler->outputData = (char*)bv->binaryData + input_accessor->byteOffset;
        sampler->outputDataCount = (int32_t)output_accessor->count;
        sampler->outputDataStride = gltf::ElementSize(output_accessor->componentType, output_accessor->elementType);
    }

    m_AnimSamplers[sampler_index] = sampler;
    return sampler;
}

template <typename T, typename N>
static void parse_json_array(const T& value, std::vector<N>& vecs)
{
    vecs.resize(value.Size());
    for (size_t i = 0; i < value.Size(); i++)
    {
        vecs[i] = value[(rapidjson::SizeType)i].template Get<N>();
    }
}

LoaderAccessorPtr glTF2_Loader::LoadAccessor(uint32_t accessor_index)
{
    RETURN_IF_FOUND(m_Accessors, accessor_index);
    
    LoaderAccessorPtr accessor = nullptr;
    if (m_Doc.HasMember("accessors"))
    {
        Value& v_accessors = m_Doc["accessors"];
        if (v_accessors.IsArray() && accessor_index < v_accessors.Size())
        {
            Value& v = v_accessors[accessor_index];
            accessor = MakeSharedPtr<LoaderAccessor>();
            if (v.HasMember("bufferView"))
                accessor->bufferViewIndex = v["bufferView"].GetInt();
            if (v.HasMember("byteOffset"))
                accessor->byteOffset = v["byteOffset"].GetInt();
            accessor->componentType = v["componentType"].GetInt();
            accessor->count = v["count"].GetInt();
            accessor->elementType = gltf::ConvertToElementType(v["type"].GetString());
            if (v.HasMember("name"))
                accessor->name = v["name"].GetString();
            if (v.HasMember("max"))
                parse_json_array(v["max"], accessor->maxValues);
            if (v.HasMember("min"))
                parse_json_array(v["min"], accessor->minValues);
            m_Accessors[accessor_index] = accessor;
        }
    }
    return accessor;
}

LoaderBufferViewPtr glTF2_Loader::LoadBufferView(uint32_t buffer_view_index)
{
    RETURN_IF_FOUND(m_BufferViews, buffer_view_index);
    
    Value& v = m_Doc["bufferViews"][buffer_view_index];
    LoaderBufferViewPtr buffer_view = MakeSharedPtr<LoaderBufferView>();
    buffer_view->bufferIndex = v["buffer"].GetUint();
    
    LoaderBufferPtr buffer = LoadBuffer(buffer_view->bufferIndex);
    if (!buffer)
    {
        LOG_ERROR("bufferView(%d) refer a nonexistent buffer(%d)", buffer_view_index, buffer_view->bufferIndex);
        return nullptr;
    }
    
    if (v.HasMember("byteStride"))
        buffer_view->byteStride = v["byteStride"].GetUint();
    if (v.HasMember("byteOffset"))
        buffer_view->byteOffset = v["byteOffset"].GetUint();
    buffer_view->byteLength = v["byteLength"].GetUint();
    if (v.HasMember("target"))
        buffer_view->target = v["target"].GetUint();
    buffer_view->binaryData = buffer->binaryData + buffer_view->byteOffset;
    
    std::shared_ptr<BufferResource> res = MakeSharedPtr<BufferResource>();
    res->_data = buffer_view->binaryData;
    res->_size = buffer_view->byteLength;
    res->RetainBackendResource(buffer->backendBuffer);
    buffer_view->backendBuffer = std::move(res);
    
    m_BufferViews[buffer_view_index] = std::move(buffer_view);
    return m_BufferViews[buffer_view_index];
}

bool IsURIData(const string& uri, string& mimeType, int32_t& prefixStrSize)
{
    string prefixStr = "data:application/octet-stream;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        return true;
    }

    prefixStr = "data:application/gltf-buffer;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        return true;
    }

    prefixStr = "data:image/png;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        mimeType = "image/png";
        return true;
    }

    prefixStr = "data:image/jpeg;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        mimeType = "image/jpeg";
        return true;
    }

#if 0
    prefixStr = "data:image/gif;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        mimeType = "image/gif";
        return true;
    }

    prefixStr = "data:image/bmp;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        mimeType = "image/bmp";
        return true;
    }

    prefixStr = "data:text/plain;base64,";
    if (uri.find(prefixStr) == 0)
    {
        prefixStrSize = int32_t(prefixStr.size());
        mimeType = "text/plain";
        return true;
    }
#endif

    return false;
}

bool DecodeDataFromURI(const char* uri, int32_t uriLen, int32_t decodeByteLen, uint8_t** binaryDataPtr)
{
    if (*binaryDataPtr || decodeByteLen == 0)
    {
        LOG_ERROR("ptr is not null");
        return false;
    }

    int dstSize = ZBase64::Decode(uri, uriLen, (char**)binaryDataPtr);

    if (!(*binaryDataPtr))
    {
        LOG_ERROR("uri data decode error");
        return false;
    }

    if(dstSize != decodeByteLen)
    {
        LOG_ERROR("uri data decode error");
        free(*binaryDataPtr);
        *binaryDataPtr = nullptr;
        return false;
    }

    return true;
}

LoaderBufferPtr& glTF2_Loader::LoadBuffer(uint32_t buffer_index)
{
    RETURN_IF_FOUND(m_Buffers, buffer_index)
    
    LoaderBufferPtr& buffer_ = m_Buffers.emplace(buffer_index, nullptr).first->second;
    LoaderBufferPtr buffer = MakeSharedPtr<LoaderBuffer>();
    
    if (!m_Doc["buffers"].IsArray() || buffer_index >= m_Doc["buffers"].Size())
        return buffer_;
    
    Value& v = m_Doc["buffers"][buffer_index];
    buffer->byteLength = v["byteLength"].GetUint();;
    buffer->binaryData = nullptr;

    bool hasUriProperty = v.HasMember("uri");
    if (!hasUriProperty)
    {
        // only first buffer in buffers can have undefined uri property
        // must be GLB and must have bin chunk
        if (!m_GLBInfo || m_GLBInfo->binChunk.chunkHeader.chunkType == 0 || buffer_index != 0)
            return buffer_;
        buffer->binaryData = m_GLBInfo->binChunk.chunkData;
        buffer->backendBuffer = m_gltfResource;
    }
    else
    {
        std::shared_ptr<BufferResource> bufferRes = MakeSharedPtr<BufferResource>();

        const char* uri = v["uri"].GetString();
        int32_t uriLen = (int32_t)v["uri"].GetStringLength();
        
        size_t uriPrefixLen = Math::Max<int32_t>(100, uriLen);
        std::string uriPrefix(uri, uriPrefixLen);
        std::string mimeType;
        int32_t prefixStrSize = 0;
        if (IsURIData(uriPrefix, mimeType, prefixStrSize))
        {
            uint8_t* decodeDataPtr = nullptr;
            if (DecodeDataFromURI(uri + prefixStrSize, v["uri"].GetStringLength() - prefixStrSize, buffer->byteLength, &decodeDataPtr))
            {
                buffer->binaryData = decodeDataPtr;
                bufferRes->_data = (uint8_t*)decodeDataPtr;
                bufferRes->_size = buffer->byteLength;
                bufferRes->_uninitializer = [decodeDataPtr](IResource*) {
                    if (decodeDataPtr)
                        free(decodeDataPtr);
                };
            }
            else
            {
                LOG_ERROR("Decode data failed in buffer %d", buffer_index);
                m_isLoadSucceed = false;
                return buffer_;
            }
        }
        else
        {
            std::string uriPath = m_filePath.substr(0, m_filePath.find_last_of('/')) + "/" + uri;
            std::shared_ptr<FileResource> uriFileRes = MakeSharedPtr<FileResource>(uriPath);
            if (!uriFileRes->IsAvailable())
            {
                LOG_ERROR("gltf_Loader: load uri file %s fail", uriPath.c_str());
                return buffer_;
            }
            bufferRes->_data = (uint8_t*)uriFileRes->_data;
            bufferRes->_size = buffer->byteLength;
            bufferRes->RetainBackendResource(std::move(uriFileRes));

            buffer->binaryData = reinterpret_cast<uint8_t*>(bufferRes->_data);
            buffer->uriPath = uriPath;
        }
        buffer->backendBuffer = std::move(bufferRes);
    }

    buffer_ = std::move(buffer);
    return buffer_;
}

void glTF2_Loader::ConverterToMorphStreamUnitFromTargets(RHIMeshPtr& m_pMesh, Value& v, AABBox& targets_box)
{
    int32_t targetSize = v.Size();
    uint32_t accessorIndex = v[0]["POSITION"].GetUint();
    LoaderAccessorPtr accessor = LoadAccessor(accessorIndex);
    vector<string> targetNames;
    bool targetNamesFlag = false;
    if (accessor->name != "")
    {
        targetNamesFlag = true;
        targetNames.resize(targetSize, "");
    }
    int32_t targetCount = int32_t(accessor->count);
    int32_t allTargetDataSize = targetSize * targetCount * 4; //float4
    //vector<float> allTargetData(allTargetDataSize, 0.0f);

    MorphTargetResource morphTargetRes;
    std::shared_ptr<float> allTargetData{ new float[allTargetDataSize]{ 0.0f }, default_array_deleter<float>() };
    morphTargetRes._uninitializer = [allTargetData](IResource*) mutable {
        allTargetData.reset();
    };
    float* allTargetDataPtr = allTargetData.get();
    for (int32_t i = 0; i < targetSize; i++)
    {
        if (v[i].HasMember("POSITION"))
        {
            accessorIndex = v[i]["POSITION"].GetUint();
            accessor = LoadAccessor(accessorIndex);
            if (accessor)
            {
                if(targetNamesFlag)  targetNames[i] = accessor->name;
                AABBox tmp_box(float3{ accessor->minValues.data() }, float3{ accessor->maxValues.data() });
                targets_box |= tmp_box;

                LoaderBufferViewPtr bv = LoadBufferView(accessor->bufferViewIndex);
                if (!bv || accessor->count != targetCount || bv->byteLength / 12 != targetCount) return;

                float* targetData = (float*)((char*)(bv->binaryData) + accessor->byteOffset);
                for (int32_t j = 0; j < targetCount; j++)
                {
                    //int32_t addr = (j * targetSize + i) * 4;
                    int32_t addr = (j * targetSize + i) * 4;
                    allTargetDataPtr[addr] = targetData[0];
                    allTargetDataPtr[addr + 1] = targetData[1];
                    allTargetDataPtr[addr + 2] = targetData[2];
                    targetData += 3;
                }
            }
        }
    }

    // RenderContext* rc = &m_pContext->RenderContextInstance();
    // RenderBufferData MorphTargetData(allTargetDataSize * sizeof(float), allTargetData.data());
    // RenderBufferPtr MorphTargetBuffer = rc->CreateStructuredBuffer(allTargetDataSize * sizeof(float), 0, sizeof(float4), &MorphTargetData);
    // MorphInfo& info = m_pMesh->GetMorphInfo();
    // info.render_buffer = MorphTargetBuffer;
    // info.morph_target_type = MorphTargetType::Position;
    // info.morph_target_weights.resize(targetSize, 0.0f);
    // info.morph_target_names = targetNames;

    morphTargetRes._morphInfo.morph_target_names = targetNames;
    morphTargetRes._morphInfo.morph_target_type = MorphTargetType::Position;
    morphTargetRes._morphInfo.morph_target_weights.resize(targetSize, 0.0f);
    morphTargetRes._data = (uint8_t*)allTargetData.get();
    morphTargetRes._size = allTargetDataSize * sizeof(float);
    m_pMesh->SetMorphTargetResource(morphTargetRes);
}

std::shared_ptr<FileResource>& glTF2_Loader::LoadUriFile(const std::string& uriFilePath)
{
    auto it = m_uriFileResources.find(uriFilePath);
    if (it != m_uriFileResources.end())
        return it->second;
    
    std::shared_ptr<FileResource> fileRes = MakeSharedPtr<FileResource>(uriFilePath);
    if (!fileRes->IsAvailable())
        fileRes = nullptr;
    
    m_uriFileResources[uriFilePath] = std::move(fileRes);
    return m_uriFileResources[uriFilePath];
}

std::map<uint32_t, MeshComponentPtr>& glTF2_Loader::GetMeshComponentMap()
{
    return m_Meshes;
}

std::map<uint32_t, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMap()
{
    return m_Nodes;
}

std::map<std::string, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMapByName()
{
    return m_NodesMapByName;
}

rapidjson::Document& glTF2_Loader::GetModelDoc()
{
    return m_Doc;
}

SEEK_NAMESPACE_END



#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
