#pragma once

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <set>
#include <string>
#include <vector>

namespace shadercompiler
{
    static const std::string PREDEFINE_PREFIX = "//PREDEFINE:";
    static const std::string STAGE_PREFIX = "//STAGE:";

    struct JsonWriterBase
    {
        JsonWriterBase(bool bTIghtJson = false)
            : m_bTightJson(bTIghtJson)
        { }

        virtual ~JsonWriterBase() {}

        int DoWrite(std::string& content)
        {
            rapidjson::StringBuffer writeBuffer;
            if (m_bTightJson)
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(writeBuffer);
                m_doc.Accept(writer);
            }
            else
            {
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(writeBuffer);
                m_doc.Accept(writer);
            }

            content = std::string{ writeBuffer.GetString(), writeBuffer.GetSize() };
            return 0;
        }

        rapidjson::Document& Document()
        {
            return m_doc;
        }

        rapidjson::Document::AllocatorType& Allocator()
        {
            return m_doc.GetAllocator();
        }

        rapidjson::Document m_doc;
        bool m_bTightJson = false;
    };

    struct JsonReaderBase
    {
        JsonReaderBase(const char* jsonBuffer, size_t jsonSize)
            : m_jsonBuffer(jsonBuffer)
            , m_jsonSize(jsonSize)
        { }

        virtual ~JsonReaderBase() {}

        int DoRead()
        {
            static constexpr uint32_t parseFlags = 
                rapidjson::kParseNoFlags |
                rapidjson::kParseCommentsFlag | 
                rapidjson::kParseTrailingCommasFlag;
            m_doc.Parse<parseFlags>(m_jsonBuffer, m_jsonSize);
            auto errorcode = m_doc.GetParseError();
            if (errorcode != rapidjson::kParseErrorNone)
                return -__LINE__;
            return 0;
        }

        rapidjson::Document& Document()
        {
            return m_doc;
        }

        rapidjson::Document::AllocatorType& Allocator()
        {
            return m_doc.GetAllocator();
        }

        rapidjson::Document m_doc;
        const char* m_jsonBuffer = nullptr;
        size_t m_jsonSize = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    ///// shader meta information
    struct MetaPredefine
    {
        std::string name;
        std::vector<std::string> candidate_values;
    };

    struct MetaInfo
    {
        std::string stage;
        std::vector<MetaPredefine> predefines;
    };

    struct MetaJsonWriter : public JsonWriterBase
    {
        MetaJsonWriter(bool bTightJson = false)
            : JsonWriterBase(bTightJson)
        { }

        int Write(const MetaInfo& metaInfo, std::string& content)
        {
            auto& root = Document().SetObject();

            rapidjson::Value metaVal(rapidjson::kObjectType);
            AddStage(metaVal, metaInfo);
            AddPredefines(metaVal, metaInfo);

            root.AddMember("meta", metaVal, Allocator());
            return DoWrite(content);
        }

        void AddStage(rapidjson::Value& parent, const MetaInfo& metaInfo)
        {
            if (metaInfo.stage.empty())
                return;

            parent.AddMember("stage", rapidjson::StringRef(metaInfo.stage.c_str(), metaInfo.stage.size()), Allocator());
        }

        void AddPredefines(rapidjson::Value& parent, const MetaInfo& metaInfo)
        {
            if (metaInfo.predefines.size() == 0)
                return;

            rapidjson::Value predefinesVal(rapidjson::kArrayType);
            for (auto& predefine : metaInfo.predefines)
            {
                rapidjson::Value predefineVal(rapidjson::kObjectType);
                predefineVal.AddMember("name", rapidjson::StringRef(predefine.name.c_str()), Allocator());
                rapidjson::Value candicateVal(rapidjson::kArrayType);
                for (auto& candicate : predefine.candidate_values)
                {
                    candicateVal.PushBack(rapidjson::StringRef(candicate.c_str()), Allocator());
                }
                predefineVal.AddMember("candidate_values", candicateVal, Allocator());
                predefinesVal.PushBack(predefineVal, Allocator());
            }
            parent.AddMember("predefines", predefinesVal, Allocator());
        }
    };

    struct MetaJsonReader : public JsonReaderBase
    {
        MetaJsonReader(const char* jsonBuffer, size_t jsonSize)
            : JsonReaderBase(jsonBuffer, jsonSize)
        { }

        int Read(MetaInfo& metaInfo)
        {
            int ret = DoRead();
            if (ret != 0)
                return ret;

            if (!m_doc.HasMember("meta"))
                return -__LINE__;

            // TODO: validate
            auto& metaVal = m_doc["meta"];
            metaInfo.stage = metaVal["stage"].GetString();
            if (metaVal.HasMember("predefines"))
            {
                auto& predefinesVal = metaVal["predefines"];
                for (uint32_t predefineIdx = 0; predefineIdx < predefinesVal.Size(); predefineIdx++)
                {
                    auto& predefineVal = predefinesVal[predefineIdx];
                    MetaPredefine predefine;
                    predefine.name = predefineVal["name"].GetString();
                    auto& candidateVal = predefineVal["candidate_values"];
                    for (uint32_t candidateIdx = 0; candidateIdx < candidateVal.Size(); candidateIdx++)
                    {
                        predefine.candidate_values.push_back(candidateVal[candidateIdx].GetString());
                    }
                    metaInfo.predefines.push_back(predefine);
                }
            }
            return 0;
        }
    };

    inline int ParseMetaPredefine(const std::string& input, MetaPredefine& predefine)
    {
        size_t pos = input.find('=');
        if (pos == std::string::npos) {
            //std::cerr << "invalid PREDEFINE input: " << input << ", PREDEFINE must have a delimiter char \'=\'" << std::endl;
            return -__LINE__;
        }

        if (pos == 0) {
            //std::cerr << "invalid PREDEFINE input: " << input << ", PREDEFINE must have a name" << std::endl;
            return -__LINE__;
        }

        predefine.name = input.substr(0, pos);

        std::string candidate_values = input.substr(pos + 1);
        size_t off = 0;
        while (true)
        {
            pos = candidate_values.find(',', off);
            if (pos == std::string::npos)
            {
                predefine.candidate_values.push_back(candidate_values.substr(off));
                break;
            }

            predefine.candidate_values.push_back(candidate_values.substr(off, pos - off));
            off = pos + 1;
        }

        std::set<std::string> _candidate_values(predefine.candidate_values.begin(), predefine.candidate_values.end());
        if (_candidate_values.size() != predefine.candidate_values.size())
        {
            //std::cerr << "invalid PREDEFINE input: " << input << ", contains duplicated value" << std::endl;
            return -__LINE__;
        }

        return 0;
    }

    inline int WriteMetaJson(const MetaInfo& metaInfo, bool bTightJson, std::string& content)
    {
        MetaJsonWriter writer(bTightJson);
        return writer.Write(metaInfo, content);
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    ///// shader reflect information
    enum class CodeType : uint8_t
    {
        SourceCode = 0,
        ByteCode,
    };

    enum class ResourceType : uint8_t
    {
        ConstantBuffer = 0,
        Texture,
        Sampler,
        SampledTexture,
        //ShaderResourceView,
        //UnorderedAccessView,
        Buffer,
        RWBuffer,
        RWTexture,
    };

    static const std::string ResourceTypeStr[] = {
        "cbuffer",
        "texture",
        "sampler",
        "sampled_texture",
        //"srv",
        //"uav",
        "buffer",
        "rwbuffer",
        "rwtexture",
    };

    struct ResourceInfo
    {
        ResourceType type;
        std::string name;
        uint32_t binding;
        uint32_t bindCount;
        uint32_t size;
        std::string fallback_name;
        std::string sampler_name;
        std::string texture_name;
    };

    struct SignatureParameter
    {
        std::string semantic;
        uint32_t semantic_index;
        uint32_t location;
    };

    struct ThreadBlockSize
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
    };

    struct ReflectInfo
    {
        CodeType code_type;
        std::string stage;
        std::string entry_point;
        std::vector<ResourceInfo> resources;
        std::vector<SignatureParameter> input_signatures;
        std::vector<SignatureParameter> output_signatures;
        ThreadBlockSize block_size; // only valid for cs, otherwise it will be {0, 0, 0}
    };

    struct ReflectJsonWriter : public JsonWriterBase
    {
        ReflectJsonWriter(bool bTightJson = false)
            : JsonWriterBase(bTightJson)
        { }

        int Write(const ReflectInfo& reflectInfo, std::string& content)
        {
            auto& root = Document().SetObject();

            rapidjson::Value reflectVal(rapidjson::kObjectType);
            reflectVal.AddMember("code_type", (uint32_t)reflectInfo.code_type, Allocator());
            reflectVal.AddMember("stage", rapidjson::StringRef(reflectInfo.stage.c_str()), Allocator());
            reflectVal.AddMember("entry_point", rapidjson::StringRef(reflectInfo.entry_point.c_str()), Allocator());
            if (reflectInfo.resources.size() > 0)
            {
                rapidjson::Value resourcesVal(rapidjson::kArrayType);
                for (uint32_t resourceIdx = 0; resourceIdx < reflectInfo.resources.size(); resourceIdx++)
                {
                    auto& resource = reflectInfo.resources[resourceIdx];
                    rapidjson::Value resourceVal(rapidjson::kObjectType);
                    resourceVal.AddMember("type", (uint32_t)resource.type, Allocator());
                    resourceVal.AddMember("name", rapidjson::StringRef(resource.name.c_str()), Allocator());
                    resourceVal.AddMember("binding", resource.binding, Allocator());
                    resourceVal.AddMember("bindCount", resource.bindCount, Allocator());
                    if (resource.size > 0)
                        resourceVal.AddMember("size", resource.size, Allocator());
                    if (!resource.fallback_name.empty())
                        resourceVal.AddMember("fallback_name", rapidjson::StringRef(resource.fallback_name.c_str()), Allocator());
                    if (!resource.sampler_name.empty())
                        resourceVal.AddMember("sampler_name", rapidjson::StringRef(resource.sampler_name.c_str()), Allocator());
                    if (!resource.texture_name.empty())
                        resourceVal.AddMember("texture_name", rapidjson::StringRef(resource.texture_name.c_str()), Allocator());
                    resourcesVal.PushBack(resourceVal, Allocator());
                }
                reflectVal.AddMember("resources", resourcesVal, Allocator());
            }
            if (reflectInfo.input_signatures.size() > 0)
            {
                rapidjson::Value inputSignatureVals(rapidjson::kArrayType);
                for (size_t idx = 0; idx != reflectInfo.input_signatures.size(); idx++)
                {
                    const SignatureParameter& inputSignature = reflectInfo.input_signatures[idx];
                    rapidjson::Value inputSignatureVal(rapidjson::kObjectType);
                    inputSignatureVal.AddMember("semantic", rapidjson::StringRef(inputSignature.semantic.c_str()), Allocator());
                    inputSignatureVal.AddMember("semantic_index", inputSignature.semantic_index, Allocator());
                    inputSignatureVal.AddMember("location", inputSignature.location, Allocator());
                    inputSignatureVals.PushBack(inputSignatureVal, Allocator());
                }
                reflectVal.AddMember("inputs", inputSignatureVals, Allocator());
            }
            if (reflectInfo.output_signatures.size() > 0)
            {
                rapidjson::Value outputSignatureVals(rapidjson::kArrayType);
                for (size_t idx = 0; idx != reflectInfo.output_signatures.size(); idx++)
                {
                    const SignatureParameter& outputSignature = reflectInfo.output_signatures[idx];
                    rapidjson::Value inputSignatureVal(rapidjson::kObjectType);
                    inputSignatureVal.AddMember("semantic", rapidjson::StringRef(outputSignature.semantic.c_str()), Allocator());
                    inputSignatureVal.AddMember("semantic_index", outputSignature.semantic_index, Allocator());
                    inputSignatureVal.AddMember("location", outputSignature.location, Allocator());
                    outputSignatureVals.PushBack(inputSignatureVal, Allocator());
                }
                reflectVal.AddMember("outputs", outputSignatureVals, Allocator());
            }
            if (reflectInfo.stage == "cs")
            {
                rapidjson::Value blockSizeVal(rapidjson::kArrayType);
                blockSizeVal.PushBack(reflectInfo.block_size.x, Allocator());
                blockSizeVal.PushBack(reflectInfo.block_size.y, Allocator());
                blockSizeVal.PushBack(reflectInfo.block_size.z, Allocator());
                reflectVal.AddMember("block_size", blockSizeVal, Allocator());
            }
            root.AddMember("reflect", reflectVal, Allocator());
            return DoWrite(content);
        }
    };

    inline int WriteReflectJson(const ReflectInfo& reflectInfo, bool bTightJson, std::string& content)
    {
        ReflectJsonWriter writer(bTightJson);
        return writer.Write(reflectInfo, content);
    }

    struct ReflectJsonReader : public JsonReaderBase
    {
        ReflectJsonReader(const char* jsonBuffer, size_t jsonSize)
            : JsonReaderBase(jsonBuffer, jsonSize)
        { }

        int Read(ReflectInfo& reflectInfo)
        {
            int ret = DoRead();
            if (ret != 0)
                return ret;

            // TODO: validate
            auto& reflectVal = m_doc["reflect"];
            reflectInfo.code_type = (CodeType)reflectVal["code_type"].GetUint();
            reflectInfo.stage = reflectVal["stage"].GetString();
            reflectInfo.entry_point = reflectVal["entry_point"].GetString();
            if (reflectVal.HasMember("resources"))
            {
                auto& resourcesVal = reflectVal["resources"];
                reflectInfo.resources.resize(resourcesVal.Size());
                for (uint32_t idx = 0; idx < resourcesVal.Size(); idx++)
                {
                    auto& resourceVal = resourcesVal[idx];
                    reflectInfo.resources[idx].type = (ResourceType)resourceVal["type"].GetUint();
                    reflectInfo.resources[idx].name = resourceVal["name"].GetString();
                    reflectInfo.resources[idx].binding = resourceVal["binding"].GetUint();
                    reflectInfo.resources[idx].bindCount = resourceVal["bindCount"].GetUint();
                    if (resourceVal.HasMember("size"))
                        reflectInfo.resources[idx].size = resourceVal["size"].GetUint();
                    if (resourceVal.HasMember("fallback_name"))
                        reflectInfo.resources[idx].fallback_name = resourceVal["fallback_name"].GetString();
                    if (resourceVal.HasMember("sampler_name"))
                        reflectInfo.resources[idx].sampler_name = resourceVal["sampler_name"].GetString();
                    if (resourceVal.HasMember("texture_name"))
                        reflectInfo.resources[idx].texture_name = resourceVal["texture_name"].GetString();
                }
            }
            if (reflectVal.HasMember("inputs"))
            {
                auto& inputSignatureVals = reflectVal["inputs"];
                reflectInfo.input_signatures.resize(inputSignatureVals.Size());
                for (uint32_t idx = 0; idx < inputSignatureVals.Size(); idx++)
                {
                    auto& inputSignatureVal = inputSignatureVals[idx];
                    reflectInfo.input_signatures[idx].semantic = inputSignatureVal["semantic"].GetString();
                    reflectInfo.input_signatures[idx].semantic_index = inputSignatureVal["semantic_index"].GetUint();
                    reflectInfo.input_signatures[idx].location = inputSignatureVal["location"].GetUint();
                }
            }
            if (reflectVal.HasMember("outputs"))
            {
                auto& outputSignatureVals = reflectVal["outputs"];
                reflectInfo.output_signatures.resize(outputSignatureVals.Size());
                for (uint32_t idx = 0; idx < outputSignatureVals.Size(); idx++)
                {
                    auto& outputSignatureVal = outputSignatureVals[idx];
                    reflectInfo.output_signatures[idx].semantic = outputSignatureVal["semantic"].GetString();
                    reflectInfo.output_signatures[idx].semantic_index = outputSignatureVal["semantic_index"].GetUint();
                    reflectInfo.output_signatures[idx].location = outputSignatureVal["location"].GetUint();
                }
            }
            if (reflectVal.HasMember("block_size"))
            {
                auto& blockSizeVal = reflectVal["block_size"];
                reflectInfo.block_size.x = blockSizeVal[0].GetUint();
                reflectInfo.block_size.y = blockSizeVal[1].GetUint();
                reflectInfo.block_size.z = blockSizeVal[2].GetUint();
            }
            return 0;
        }
    };
    
} // namespace shadercompiler
