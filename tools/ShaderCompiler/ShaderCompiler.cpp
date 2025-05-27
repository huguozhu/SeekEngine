#include "seek.config.h"
#include "ShaderConductor/ShaderConductor.hpp"
#include "shader_helper.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#if defined(_WIN32)
#include <windows.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#endif
#include "CLI/CLI.hpp"
#include "shader_content_def.h"

using namespace ShaderConductor;
using namespace shadercompiler;

#define SUCCESS     0
#define FAIL        (-__LINE__)

#define PRIME_NUM 0x9e3779b9

template <typename T>
inline void HashCombineImpl(T& seed, T value)
{
    seed ^= value + PRIME_NUM + (seed << 6) + (seed >> 2);
}

template <typename T>
inline size_t HashValue(T v)
{
    return static_cast<size_t>(v);
}

template <typename T>
inline size_t HashValue(T* v)
{
    return static_cast<size_t>(reinterpret_cast<intptr_t>(v));
}

template <typename T>
inline void HashCombine(size_t& seed, T const& v)
{
    return HashCombineImpl(seed, HashValue(v));
}

template <typename T>
inline void HashRange(size_t& seed, T first, T last)
{
    for (; first != last; ++first)
    {
        HashCombine(seed, *first);
    }
}

template <typename T>
inline size_t HashRange(T first, T last)
{
    size_t seed = 0;
    HashRange(seed, first, last);
    return seed;
}

static void ParseReflectInfo(const Reflection& m_reflection, const std::string& stageName, bool isByteCode, ReflectInfo& reflectInfo)
{
    reflectInfo.stage = stageName;
    reflectInfo.entry_point = "main";
    if (isByteCode)
        reflectInfo.code_type = CodeType::ByteCode;
    else
        reflectInfo.code_type = CodeType::SourceCode;
    
    for (uint32_t resourceIdx = 0; resourceIdx < m_reflection.NumResources(); resourceIdx++)
    {
        auto resource = m_reflection.ResourceByIndex(resourceIdx);

        ResourceInfo resourceInfo;
        resourceInfo.binding = resource->bindPoint;
        resourceInfo.bindCount = resource->bindCount;
        resourceInfo.name = resource->name;
        resourceInfo.fallback_name = resource->fallbackName;
        resourceInfo.sampler_name = resource->samplerName;
        resourceInfo.texture_name = resource->textureName;
        resourceInfo.size = 0;
        switch (resource->type)
        {
            case ShaderResourceType::ConstantBuffer:
            {    
                resourceInfo.type = ResourceType::ConstantBuffer;
                auto constant_buffer = m_reflection.ConstantBufferByName(resource->name);
                resourceInfo.size = constant_buffer->Size();
                break;
            }
            case ShaderResourceType::Texture:
                resourceInfo.type = ResourceType::Texture;
                break;
            case ShaderResourceType::Sampler:
                resourceInfo.type = ResourceType::Sampler;
                break;
            case ShaderResourceType::SampledTexture:
                resourceInfo.type = ResourceType::SampledTexture;
                break;
            case ShaderResourceType::Buffer:
                resourceInfo.type = ResourceType::Buffer;
                break;
            case ShaderResourceType::RWBuffer:
                resourceInfo.type = ResourceType::RWBuffer;
                break;
            case ShaderResourceType::RWTexture:
                resourceInfo.type = ResourceType::RWTexture;
                break;
            default:
                continue; // ignore other resource
        }
        reflectInfo.resources.push_back(resourceInfo);
    }

    for (uint32_t idx = 0; idx < m_reflection.NumInputParameters(); idx++)
    {
        auto inputParameterDesc = m_reflection.InputParameter(idx);
        SignatureParameter signatureParameter;
        signatureParameter.semantic = inputParameterDesc->semantic;
        signatureParameter.semantic_index = inputParameterDesc->semanticIndex;
        signatureParameter.location = inputParameterDesc->location;
        reflectInfo.input_signatures.push_back(signatureParameter);
    }

    for (uint32_t idx = 0; idx < m_reflection.NumOutputParameters(); idx++)
    {
        auto outputParameterDesc = m_reflection.OutputParameter(idx);
        SignatureParameter signatureParameter;
        signatureParameter.semantic = outputParameterDesc->semantic;
        signatureParameter.semantic_index = outputParameterDesc->semanticIndex;
        signatureParameter.location = outputParameterDesc->location;
        reflectInfo.output_signatures.push_back(signatureParameter);
    }

    if (stageName == "cs")
    {
        reflectInfo.block_size.x = m_reflection.CSBlockSizeX();
        reflectInfo.block_size.y = m_reflection.CSBlockSizeY();
        reflectInfo.block_size.z = m_reflection.CSBlockSizeZ();
    }
}

static MetaInfo _shaderMetaInfo;

struct ExtentPredefine
{
    ExtentPredefine(const MetaPredefine& metaPredefine)
        : name(metaPredefine.name)
        , candidate_values(metaPredefine.candidate_values)
    { }

    ExtentPredefine() {}

    std::string name;
    std::string value;
    std::vector<std::string> candidate_values;
};

static inline ShaderStage ParseShaderStage(const std::string& stageName)
{
    if (stageName == "vs")
    {
        return ShaderStage::VertexShader;
    }
    else if (stageName == "ps")
    {
        return ShaderStage::PixelShader;
    }
    //else if (stageName == "gs")
    //{
    //    return ShaderStage::GeometryShader;
    //}
    //else if (stageName == "hs")
    //{
    //    return ShaderStage::HullShader;
    //}
    //else if (stageName == "ds")
    //{
    //    return ShaderStage::DomainShader;
    //}
    else if (stageName == "cs")
    {
        return ShaderStage::ComputeShader;
    }
    else
    {
        return ShaderStage::NumShaderStages;
    }
}

static inline ShadingLanguage ParseShadingLanguage(const std::string& targetName)
{
    if (targetName == "dxil")
    {
        return ShadingLanguage::Dxil;
    }
    else if (targetName == "spirv")
    {
        return ShadingLanguage::SpirV;
    }
    else if (targetName == "hlsl")
    {
        return ShadingLanguage::Hlsl;
    }
    else if (targetName == "glsl")
    {
        return ShadingLanguage::Glsl;
    }
    else if (targetName == "essl")
    {
        return ShadingLanguage::Essl;
    }
    else if (targetName == "msl_macos")
    {
        return ShadingLanguage::Msl_macOS;
    }
    else if (targetName == "msl_ios")
    {
        return ShadingLanguage::Msl_iOS;
    }
    else
    {
        return ShadingLanguage::NumShadingLanguages;
    }
}

static ShaderStage ParseShaderStageFromFileExtension(const std::string& ext)
{
    if (ext == "vs")
        return ShaderStage::VertexShader;
    else if (ext == "ps")
        return ShaderStage::PixelShader;
    else if (ext == "cs")
        return ShaderStage::ComputeShader;
    else if (ext == "gs")
        return ShaderStage::GeometryShader;
    else if (ext == "hs")
        return ShaderStage::HullShader;
    else if (ext == "ds")
        return ShaderStage::DomainShader;
    else
        return ShaderStage::NumShaderStages;
}

static std::map<ShadingLanguage, std::string> shaderLanguageMap{
    {ShadingLanguage::Dxil, "dxil"},
    {ShadingLanguage::SpirV, "spriv"},
    {ShadingLanguage::Hlsl, "hlsl"},
    {ShadingLanguage::Glsl, "glsl"},
    {ShadingLanguage::Essl, "essl"},
    {ShadingLanguage::Msl_macOS, "msl_macos"},
    {ShadingLanguage::Msl_iOS, "msl_ios"},
};

static std::map<ShadingLanguage, std::string> shaderLanguageExtMap{
    {ShadingLanguage::Dxil, ".dxil"},
    {ShadingLanguage::SpirV, ".spirv"},
    {ShadingLanguage::Hlsl, ".hlsl"},
    {ShadingLanguage::Glsl, ".glsl"},
    {ShadingLanguage::Essl, ".glsl"},
    {ShadingLanguage::Msl_macOS, ".msl"},
    {ShadingLanguage::Msl_iOS, ".msl"},
};

static std::map<ShadingLanguage, const char*> shaderLanguageVersionMap{
    {ShadingLanguage::Dxil, "60"},
    {ShadingLanguage::SpirV, nullptr},
    {ShadingLanguage::Hlsl, "50"},
    {ShadingLanguage::Glsl, "430"},
    {ShadingLanguage::Essl, "310"},
    {ShadingLanguage::Msl_macOS, nullptr},
    {ShadingLanguage::Msl_iOS, nullptr},
};

static std::map<ShaderStage, std::string> d3dShaderStageMap{
    {ShaderStage::VertexShader, "vs_5_0"},
    {ShaderStage::PixelShader, "ps_5_0"},
    {ShaderStage::GeometryShader, "gs_5_0"},
    {ShaderStage::HullShader, "hs_5_0"},
    {ShaderStage::DomainShader, "ds_5_0"},
    {ShaderStage::ComputeShader, "cs_5_0"},
};

static int read_file_content(const std::string& filename, std::string& content)
{
    std::ifstream readFile(filename, std::ios_base::binary);
    if (!readFile)
        return FAIL;

    readFile.seekg(0, std::ios::end);
    content.resize(static_cast<size_t>(readFile.tellg()));
    readFile.seekg(0, std::ios::beg);
    readFile.read((char*)content.data(), content.size());
    return SUCCESS;
}

static inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return ch != ' ';
        }));
}

static inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return ch != ' ' && ch != '\n' && ch != '\r';
        }).base(), s.end());
}

static inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

static inline std::string ltrim_copy(std::string s)
{
    ltrim(s);
    return s;
}

static inline std::string rtrim_copy(std::string s)
{
    rtrim(s);
    return s;
}

static inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}

struct IncludeFilePool
{
    const Blob& load_include_file(const std::string& includeFilePath, size_t* hash)
    {
        auto blobIt = _fileBlob.find(includeFilePath);
        if (blobIt != _fileBlob.end())
            return blobIt->second;

        std::string fileContent;
        int success = read_file_content(std::string{ SEEK_SHADER_SOURCE_DIR "/" } + includeFilePath, fileContent);
        std::cout << "load include file: " << includeFilePath << ", success: " << success << std::endl;
        if (SUCCESS != success)
            return _emptyBlob;

        if (hash)
        {
            *hash = std::hash<std::string>{}(fileContent);
        }

        return _fileBlob[includeFilePath] = Blob{ fileContent.data(), (uint32_t)fileContent.size() };
    }

    bool is_loaded(const std::string& includeFilePath)
    {
        return _fileBlob.find(includeFilePath) != _fileBlob.end();
    }

    std::map<std::string, Blob> _fileBlob;
    Blob _emptyBlob;
};

static IncludeFilePool __includeFilePool;

#if defined(_WIN32)
static int HLSLPrecompile(const void* shaderData, uint32_t shaderSize, const std::string& entryPoint, const std::string& stage, std::string& byteCode)
{
    Microsoft::WRL::ComPtr<ID3DBlob> pError;
    Microsoft::WRL::ComPtr<ID3DBlob> pCode;
    static const char* v = "vs_5_0";
    static const char* p = "ps_5_0";
    static const char* g = "gs_5_0";
    static const char* h = "hs_5_0";
    static const char* d = "ds_5_0";
    static const char* c = "cs_5_0";

    const char* target;
    if (stage == "vs")
        target = "vs_5_0";
    else if (stage == "ps")
        target = "ps_5_0";
    else if (stage == "cs")
        target = "cs_5_0";
    else
    {
        std::cerr << "unsupported stage " << stage << std::endl;
        return FAIL;
    }

    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    HRESULT hr = D3DCompile(shaderData, shaderSize, 
        nullptr, // pSourceName
        nullptr, // pDefines
        nullptr, // pInclude
        entryPoint.c_str(), // pEntryPoint
        target, // pTarget
        compileFlags, // Flags1
        0, // Flags2
        pCode.GetAddressOf(), 
        pError.GetAddressOf());

    if (hr != S_OK)
    {
        std::cerr << "D3DCompile fail, hr:" << hr << std::endl;
        if (pError)
        {
            std::cerr << "ERROR: " << std::string{ (char*)pError->GetBufferPointer() } << std::endl;
        }
        return FAIL;
    }

    if (pError)
    {
        std::cout << "WARNING: " << std::string{ (char*)pError->GetBufferPointer() } << std::endl;
    }

    byteCode.resize(pCode->GetBufferSize());
    memcpy((void*)byteCode.data(), pCode->GetBufferPointer(), pCode->GetBufferSize());
    return SUCCESS;
}
#endif // _WIN32

static int ShaderPrecompile(ShadingLanguage language, const void* shaderData, uint32_t shaderSize, const std::string& entryPoint, const std::string& stage, std::string& byteCode)
{
    switch (language)
    {
#if defined(_WIN32)
        case ShadingLanguage::Hlsl:
            return HLSLPrecompile(shaderData, shaderSize, entryPoint, stage, byteCode);
#endif
        default:
            std::cerr << "cannot precompile %s" << shaderLanguageMap[language] << std::endl;
            return FAIL;
    }
}

static void WriteByteArray(std::ofstream& outputHeaderFile, const void* data, size_t size, const std::string& varName)
{
    const uint8_t* data_ = (const uint8_t*)data;
    outputHeaderFile
        << "static const uint8_t " << varName << "[] = " << std::endl
        << "{" << std::endl;

    static const size_t perLineCnt = 8;
    size_t wholeLines = size / perLineCnt;
    for (size_t idx = 0; idx < wholeLines; idx++)
    {
        outputHeaderFile << "    " << std::hex
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 0] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 1] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 2] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 3] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 4] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 5] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 6] << ", "
            << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx * perLineCnt + 7] << ", "
            << std::endl;
    }

    size_t lastOffset = wholeLines * perLineCnt;
    size_t lastCnt = size - lastOffset;
    for (size_t idx = 0; idx < lastCnt; idx++)
    {
        if (idx == 0)
            outputHeaderFile << "    ";
        outputHeaderFile << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)data_[idx + lastOffset];
        if (idx != lastCnt - 1)
            outputHeaderFile << ", ";
        else
            outputHeaderFile << std::endl;
    }

    outputHeaderFile << "};" << std::endl;
}

void WriteMacroComment(std::ofstream& file, const std::vector<ExtentPredefine>& fixed_macros, const std::vector<ExtentPredefine>& active_macros)
{
    if (fixed_macros.size() > 0)
    {
        file << "// FIXED MACROS:" << std::endl;
        for (size_t idx = 0; idx != fixed_macros.size(); idx++)
        {
            file << "//   #define " << fixed_macros[idx].name << " " << fixed_macros[idx].value << std::endl;
        }
    }

    if (active_macros.size() > 0)
    {
        file << "// PREDEFINE MACROS:" << std::endl;
        for (size_t idx = 0; idx != active_macros.size(); idx++)
        {
            file << "//   #define " << active_macros[idx].name << " " << active_macros[idx].value << std::endl;
        }
    }
}

static int GenerateTagFile(const std::string& tagFilePath)
{
    std::ofstream tagFile(tagFilePath, std::ios_base::binary);
    if (!tagFile)
    {
        std::cerr << "couldn't open the tag file: " << tagFilePath << std::endl;
        return FAIL;
    }
    tagFile << "!!!AUTO GENERATED!!!" << std::endl
        << "it's a dummy file, to make incremental build works" << std::endl;
    return SUCCESS;
}

int main(int argc, char** argv)
{
    std::cout << "command line:";
    for (int i = 0; i < argc; i++)
    {
        std::cout << " " << argv[i];
    }
    std::cout << std::endl;

    CLI::App app{ "A tool for seek to compile HLSL to many other shader languages", "Shader Compiler" };

    std::string inputFileName;
    app.add_option("--input", inputFileName, "input shader file")
        ->required();

    std::string stageName;
    app.add_option("--stage", stageName, "shader stage")
        ->default_val("vs")
        ->run_callback_for_default()
        ->check(CLI::IsMember({ "vs", "ps", "cs" }));

    std::string entryPoint;
    app.add_option("--entry", entryPoint, "shader entry point")
        ->default_val("main")
        ->run_callback_for_default();

    std::vector<std::string> defines;
    app.add_option("--define", defines, "macro definitions, format: define=value or define");

    bool generateDebugFile = false;
    app.add_flag("--debug", generateDebugFile, "also generate debuggable shader file(no optimize, more readable to debug)");

    std::vector<std::string> targetName;
    app.add_option("--target", targetName, "target platforms to generate, format: t1;t2;t3")
        ->delimiter(';')
        ->check(CLI::IsMember({ "hlsl", "essl", "msl_macos", "msl_ios"}));

    bool generateByteCode = false;
    app.add_flag("--bytecode", generateByteCode, "generate the compiled IR bytecode in header file");

    bool generateDependFile = false;
    // when --depend is specified, only run preprocess, no compile. ignore other options except --define
    app.add_flag("--depend", generateDependFile, "generate depend file for a shader source file instead of source code file");

    bool tightJson = false;
    app.add_flag("--tightjson", tightJson, "generate tighted json for all output json file");

    CLI11_PARSE(app, argc, argv);

    std::cout << "parsed configs: " << std::endl << app.config_to_str();

    try
    {
        bool generateDebugShaderPass = false;
        std::vector<ExtentPredefine> extPredefines;
        std::string shaderSourceDir = SEEK_SHADER_SOURCE_DIR;
        std::string shaderGenerateDir = SEEK_GENERATED_SHADER_DIR;

        size_t delimiterPos = inputFileName.find_last_of('.');
        const auto inputFileBaseName = inputFileName.substr(0, delimiterPos);
        const auto inputFilePath = shaderSourceDir + "/" + inputFileName;

        // Compiler::SourceDesc
        Compiler::SourceDesc sourceDesc{};
        sourceDesc.entryPoint = entryPoint.c_str();
        sourceDesc.fileName = inputFilePath.c_str();
        std::string source;
        {
            std::ifstream inputFile(sourceDesc.fileName, std::ios_base::binary);
            if (!inputFile)
            {
                std::cerr << "couldn't open the input file: " << sourceDesc.fileName << std::endl;
                return FAIL;
            }

            inputFile.seekg(0, std::ios::end);
            source.reserve(static_cast<size_t>(inputFile.tellg()));
            inputFile.seekg(0, std::ios::beg);

            // parse meta information
            std::string lineStr;
            while (std::getline(inputFile, lineStr))
            {
                if (lineStr.find(PREDEFINE_PREFIX) == 0)
                {
                    auto predefineStr = trim_copy(lineStr.substr(PREDEFINE_PREFIX.size()));
                    MetaPredefine predefine;
                    if (ParseMetaPredefine(predefineStr, predefine) != SUCCESS)
                        return FAIL;
                    _shaderMetaInfo.predefines.push_back(predefine);
                    extPredefines.push_back(ExtentPredefine{ predefine });
                }
                else if (lineStr.find(STAGE_PREFIX) == 0)
                {
                    stageName = trim_copy(lineStr.substr(STAGE_PREFIX.size()));
                }

                source += lineStr + '\n'; // std::getline use '\n' as delimiter, so need to put it back
            }
        }
        _shaderMetaInfo.stage = stageName;

        sourceDesc.source = source.c_str();
        sourceDesc.stage = ParseShaderStage(stageName);
        if (sourceDesc.stage == ShaderStage::NumShaderStages)
        {
            std::cerr << "invalid shader stage: " << stageName << std::endl;
            return FAIL;
        }

        std::map<std::string, std::string> dependIncludeFiles;
        auto includeFileHandleFunc = [&](const char* includeFileName)->ShaderConductor::Blob
        {
            if (__includeFilePool.is_loaded(includeFileName))
                return __includeFilePool.load_include_file(includeFileName, nullptr);

            size_t hash;
            auto& blob = __includeFilePool.load_include_file(includeFileName, &hash);
            if (generateDependFile)
            {
                dependIncludeFiles[includeFileName] = std::to_string(hash);
            }
            return blob;
        };

        std::vector<ExtentPredefine> fixed_macros;
        if (defines.size() > 0)
        {
            fixed_macros.reserve(defines.size());
            for (const auto& define : defines)
            {
                size_t splitPosition = define.find('=');
                if (splitPosition != std::string::npos)
                {
                    std::string macroName = define.substr(0, splitPosition);
                    std::string macroValue = define.substr(splitPosition + 1, define.size() - splitPosition - 1);
                    ExtentPredefine _predefine;
                    _predefine.name = define.substr(0, splitPosition);
                    _predefine.value = define.substr(splitPosition + 1, define.size() - splitPosition - 1);
                    fixed_macros.emplace_back(_predefine);
                }
                else
                {
                    ExtentPredefine _predefine;
                    _predefine.name = define;
                    fixed_macros.emplace_back(_predefine);
                }
            }
        }

        // Compiler::Options
        Compiler::Options compileOptions{};
        compileOptions.packMatricesInRowMajor = false;
        compileOptions.optimizationLevel = 3;
        if (inputFileName == "FxaaPS.dsf")
            compileOptions.optimizationLevel = 0;
        compileOptions.disableOptimizations = false;
        compileOptions.enableDebugInfo = false;
        compileOptions.inheritCombinedSamplerBindings = true;
        compileOptions.hlslAutoBindResource = false;
        //compileOptions.mslEnableDecorationBinding = true;

        // Compiler::TargetDesc
        std::vector<Compiler::TargetDesc> targetDesc;
        targetDesc.resize(targetName.size());
        for (size_t outputIdx = 0; outputIdx != targetName.size(); outputIdx++)
        {
            targetDesc[outputIdx].language = ParseShadingLanguage(targetName[outputIdx]);
            if (targetDesc[outputIdx].language == ShadingLanguage::NumShadingLanguages)
            {
                std::cerr << "invalid target shading language: " << targetName[outputIdx] << std::endl;
                return FAIL;
            }
            targetDesc[outputIdx].version = shaderLanguageVersionMap[targetDesc[outputIdx].language];
        }

        using CompileFunc = std::function<int(const std::vector<ExtentPredefine>&, const std::vector<ExtentPredefine>&)>;
        using PredefineIt = std::vector<ExtentPredefine>::iterator;
        using IterateFunc = std::function<int(PredefineIt, PredefineIt, PredefineIt, const std::vector<ExtentPredefine>&, std::vector<ExtentPredefine>&, const CompileFunc&)>;

        IterateFunc iterateFunc; // iterateFunc call self recursive, so we need to define it in a single line
        iterateFunc = [&iterateFunc](PredefineIt begin, PredefineIt end, PredefineIt current,
            const std::vector<ExtentPredefine>& fixed_macros, std::vector<ExtentPredefine>& active_macros, const CompileFunc& compileFunc)->int
        {
            if (current == end)
            {
                return compileFunc(fixed_macros, active_macros);
            }

            PredefineIt next = current + 1;
            for (size_t i = 0; i < current->candidate_values.size(); i++)
            {
                active_macros[current - begin].name = current->name;
                active_macros[current - begin].value = current->candidate_values[i];

                int success = iterateFunc(begin, end, next, fixed_macros, active_macros, compileFunc);
                if (success != SUCCESS)
                    return success;
            }
            return SUCCESS;
        };

        std::vector<std::string> allOutFileNames;
        std::vector<std::string> allShaderCodeVarNames;
        std::vector<std::string> allShaderReflectVarNames;
        std::vector<std::string> allShaderCodeLanguageNames;
        auto compileFunc = [&](const std::vector<ExtentPredefine>& fixed_macros, const std::vector<ExtentPredefine>& active_macros)->int
        {
            std::vector<MacroDefine> macroDefines;
            macroDefines.reserve(fixed_macros.size() + active_macros.size());
            for (auto& macro : fixed_macros)
            {
                macroDefines.emplace_back(MacroDefine{ macro.name.c_str(), macro.value.c_str() });
            }
            std::map<std::string, std::string> activeMacroMap;
            for (auto& macro : active_macros)
            {
                macroDefines.emplace_back(MacroDefine{ macro.name.c_str(), macro.value.c_str() });
                activeMacroMap[macro.name] = macro.value;
            }

            size_t seed = 0;
            {
                for (auto mapIt : activeMacroMap)
                {
                    HashRange(seed, mapIt.second.begin(), mapIt.second.end());
                }
            }
            sourceDesc.numDefines = (uint32_t)macroDefines.size();
            sourceDesc.defines = macroDefines.data();
            std::vector<Compiler::ResultDesc> result(targetName.size());
            Compiler::Compile(sourceDesc, compileOptions, targetDesc.data(), (uint32_t)targetDesc.size(), &result[0]);

            int compileRet = SUCCESS;
            for (size_t resultIdx = 0; resultIdx != result.size(); resultIdx++)
            {
                if (result[resultIdx].errorWarningMsg.Size() > 0)
                {
                    const char* msg = reinterpret_cast<const char*>(result[resultIdx].errorWarningMsg.Data());
                    std::cerr << "Compiler::Compile errorWarningMsg: " << std::string(msg, msg + result[resultIdx].errorWarningMsg.Size()) << std::endl;
                }

                if (result[resultIdx].hasError)
                    return FAIL;

                if (result[resultIdx].target.Size() == 0)
                {
                    std::cerr << "size of compiled shader is 0, something wrong" << std::endl;
                    return FAIL;
                }

                const void* shaderSourceData = result[resultIdx].target.Data();
                uint32_t shaderSourceSize = result[resultIdx].target.Size();
                
                const void* shaderHeaderData = shaderSourceData;
                uint32_t shaderHeaderSize = shaderSourceSize;
                std::string byteCodeContent;
                if (generateByteCode)
                {
                    int ret = ShaderPrecompile(targetDesc[resultIdx].language, shaderSourceData, shaderSourceSize, entryPoint, stageName, byteCodeContent);
                    if (ret != SUCCESS)
                        return ret;

                    shaderHeaderData = byteCodeContent.data();
                    shaderHeaderSize = (uint32_t)byteCodeContent.size();
                }

                std::string outputFileDir = shaderGenerateDir + "/" + targetName[resultIdx];
                if (generateDebugShaderPass)
                    outputFileDir += SHADER_DEBUG_DIR_SUFFIX;
                
                std::string outputFileName = inputFileBaseName;
                if (seed != 0)
                {
                    std::stringstream seedStream;
                    seedStream << "_" << std::hex << seed;
                    outputFileName += seedStream.str();
                }

                ///////////////////// shader code - source file /////////////////////
                std::string outShaderSourceFilePath = outputFileDir + "/" + outputFileName + shaderLanguageExtMap[targetDesc[resultIdx].language];
                std::ofstream outShaderSourceFile(outShaderSourceFilePath, std::ios_base::binary);
                if (!outShaderSourceFile)
                {
                    std::cerr << "couldn't open file: " << outShaderSourceFilePath << std::endl;
                    return FAIL;
                }
                WriteMacroComment(outShaderSourceFile, fixed_macros, active_macros);
                outShaderSourceFile.write(reinterpret_cast<const char*>(shaderSourceData), shaderSourceSize);
                std::cout << "shader code - source file saved in: " << outShaderSourceFilePath << std::endl;

                ///////////////////// shader code - header file /////////////////////
                std::string outShaderHeaderFilePath = outputFileDir + "/" + outputFileName + ".hpp";
                std::ofstream outShaderHeaderFile(outShaderHeaderFilePath, std::ios_base::binary);
                if (!outShaderHeaderFile)
                {
                    std::cerr << "couldn't open file: " << outShaderHeaderFilePath << std::endl;
                    return FAIL;
                }
                WriteMacroComment(outShaderHeaderFile, fixed_macros, active_macros);
                outShaderHeaderFile << "#pragma once" << std::endl
                                    << "#include <cstdint>" << std::endl
                                    << std::endl;

                std::string varShaderCodeName;
                if (generateDebugShaderPass)
                    varShaderCodeName = shader_code_name(outputFileName, shaderLanguageMap[targetDesc[resultIdx].language], SHADER_DEBUG_DIR_SUFFIX);
                else
                    varShaderCodeName = shader_code_name(outputFileName, shaderLanguageMap[targetDesc[resultIdx].language], "");
                allOutFileNames.push_back(outputFileName);
                allShaderCodeVarNames.push_back(varShaderCodeName);
                allShaderCodeLanguageNames.push_back(shaderLanguageMap[targetDesc[resultIdx].language]);
                WriteByteArray(outShaderHeaderFile, shaderHeaderData, shaderHeaderSize, varShaderCodeName);
                std::cout << "shader code - header file saved in: " << outShaderHeaderFilePath << std::endl;

                ///////////////////// shader reflect - source file /////////////////////
                std::string outReflectSourceFilePath = outputFileDir + "/" + outputFileName + SHADER_REFLECT_FILE_SUFFIX;
                ReflectInfo reflectInfo;
                ParseReflectInfo(result[resultIdx].reflection, stageName, generateByteCode, reflectInfo);
                std::string reflectJsonContent;
                int wr = WriteReflectJson(reflectInfo, tightJson, reflectJsonContent);
                if (wr != 0)
                {
                    std::cerr << "write shader reflect json fail, ret: " << wr << std::endl;
                    return FAIL;
                }

                std::ofstream outReflectSourceFile(outReflectSourceFilePath, std::ios_base::binary);
                if (!outReflectSourceFile)
                {
                    std::cerr << "couldn't open file: " << outReflectSourceFilePath << std::endl;
                    return FAIL;
                }
                outReflectSourceFile << reflectJsonContent << std::endl;
                std::cout << "shader reflect - source file saved in: " << outReflectSourceFilePath << std::endl;

                ///////////////////// shader reflect - header file /////////////////////
                // shader reflect header file use the same file with shader code
                std::string varShaderReflectName;
                if (generateDebugShaderPass)
                    varShaderReflectName = shader_reflect_name(outputFileName, shaderLanguageMap[targetDesc[resultIdx].language], SHADER_DEBUG_DIR_SUFFIX);
                else
                    varShaderReflectName = shader_reflect_name(outputFileName, shaderLanguageMap[targetDesc[resultIdx].language], "");
                allShaderReflectVarNames.push_back(varShaderReflectName);
                WriteByteArray(outShaderHeaderFile, reflectJsonContent.data(), reflectJsonContent.size(), varShaderReflectName);
                std::cout << "shader reflect - header file saved in: " << outShaderHeaderFilePath << std::endl;
            }

            return compileRet;
        };

        auto preprocessFunc = [&](const std::vector<ExtentPredefine>& fixed_macros, const std::vector<ExtentPredefine>& active_macros)->int
        {
            std::vector<MacroDefine> macroDefines;
            macroDefines.reserve(fixed_macros.size() + active_macros.size());
            for (auto& macro : fixed_macros)
            {
                macroDefines.emplace_back(MacroDefine{ macro.name.c_str(), macro.value.c_str() });
            }
            sourceDesc.numDefines = (uint32_t)macroDefines.size();
            sourceDesc.defines = macroDefines.data();

            Compiler::ResultDesc result = Compiler::Preprocess(sourceDesc);

            return SUCCESS;
        };

        // compile and preprocess is mutual exclusion
        if (!generateDependFile)
        {
            compileOptions.needReflection = true;
            int success = iterateFunc(extPredefines.begin(), extPredefines.end(), extPredefines.begin(), fixed_macros, extPredefines, compileFunc);
            if (success != SUCCESS)
                return success;

            for (size_t targetIdx = 0; targetIdx != targetName.size(); targetIdx++)
            {
                std::string headerFilePath = shaderGenerateDir + "/" + targetName[targetIdx] + "/" + inputFileBaseName + ".h";
                std::ofstream headerFile(headerFilePath, std::ios_base::binary);
                if (!headerFile)
                {
                    std::cerr << "couldn't open file: " << headerFilePath << std::endl;
                    return FAIL;
                }
                headerFile << "#pragma once" << std::endl;
                headerFile << "#include \"shader_content_def.h\"" << std::endl;
                for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                {
                    headerFile << "#include \"" << allOutFileNames[idx] << ".hpp\"" << std::endl;
                }
                headerFile << std::endl;
                headerFile << "#define SHADER_CODE_MAP_GROUP_" << shaderLanguageMap[targetDesc[targetIdx].language] << "_" << inputFileBaseName << " \\" << std::endl;
                for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                {
                    headerFile << "    SHADER_CODE_MAP_MEMBER_VAR(" << allOutFileNames[idx] << "," << allShaderCodeVarNames[idx] << ") \\"<< std::endl;
                }
                headerFile << std::endl;
                headerFile << "#define SHADER_REFLECT_MAP_GROUP_" << shaderLanguageMap[targetDesc[targetIdx].language] << "_" << inputFileBaseName << " \\" << std::endl;
                for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                {
                    headerFile << "    SHADER_REFLECT_MAP_MEMBER_VAR(" << allOutFileNames[idx] << "," << allShaderReflectVarNames[idx] << ") \\"<< std::endl;
                }
                headerFile << std::endl;
            }

            if (generateDebugFile)
            {
                generateDebugShaderPass = true;
                compileOptions.optimizationLevel = 0;
                compileOptions.disableOptimizations = true;
                compileOptions.enableDebugInfo = true;
                
                allOutFileNames.clear();
                allShaderCodeVarNames.clear();
                allShaderReflectVarNames.clear();
                
                success = iterateFunc(extPredefines.begin(), extPredefines.end(), extPredefines.begin(), fixed_macros, extPredefines, compileFunc);
                if (success != SUCCESS)
                    return success;

                for (size_t targetIdx = 0; targetIdx != targetName.size(); targetIdx++)
                {
                    std::string headerFilePath = shaderGenerateDir + "/" + targetName[targetIdx] + SHADER_DEBUG_DIR_SUFFIX + "/" + inputFileBaseName + ".h";
                    std::ofstream headerFile(headerFilePath, std::ios_base::binary);
                    if (!headerFile)
                    {
                        std::cerr << "couldn't open file: " << headerFilePath << std::endl;
                        return FAIL;
                    }
                    headerFile << "#pragma once" << std::endl;
                    for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                    {
                        headerFile << "#include \"" << allOutFileNames[idx] << ".hpp\"" << std::endl;
                    }
                    headerFile << std::endl;
                    headerFile << "#define SHADER_CODE_MAP_GROUP_" << shaderLanguageMap[targetDesc[targetIdx].language] << "_" << inputFileBaseName << SHADER_DEBUG_DIR_SUFFIX << " \\" << std::endl;
                    for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                    {
                        headerFile << "    SHADER_CODE_MAP_MEMBER_VAR(" << allOutFileNames[idx] << "," << allShaderCodeVarNames[idx] << ") \\"<< std::endl;
                    }
                    headerFile << std::endl;
                    headerFile << "#define SHADER_REFLECT_MAP_GROUP_" << shaderLanguageMap[targetDesc[targetIdx].language] << "_" << inputFileBaseName << SHADER_DEBUG_DIR_SUFFIX << " \\" << std::endl;
                    for (size_t idx = 0; idx != allOutFileNames.size(); idx++)
                    {
                        headerFile << "    SHADER_REFLECT_MAP_MEMBER_VAR(" << allOutFileNames[idx] << "," << allShaderReflectVarNames[idx] << ") \\"<< std::endl;
                    }
                    headerFile << std::endl;
                }
            }

            std::string metaSourceFilePath = SEEK_GENERATED_META_DIR "/" + inputFileBaseName + SHADER_META_FILE_SUFFIX;
            std::string metaHeaderFilePath = SEEK_GENERATED_META_DIR "/" + inputFileBaseName + ".h";

            ///////////////////// shader meta - source file /////////////////////
            std::string metaJsonContent;
            int wr = WriteMetaJson(_shaderMetaInfo, tightJson, metaJsonContent);
            if (wr != 0)
                return wr;

            std::ofstream metaSourceFile(metaSourceFilePath, std::ios_base::binary);
            if (!metaSourceFile)
            {
                std::cerr << "couldn't open file: " << metaSourceFilePath << std::endl;
                return FAIL;
            }
            metaSourceFile << metaJsonContent << std::endl;
            std::cout << "shader meta - source file saved in: " << metaSourceFilePath << std::endl;

            std::ofstream metaHeaderFile(metaHeaderFilePath, std::ios_base::binary);
            if (!metaHeaderFile)
            {
                std::cerr << "couldn't open file: " << metaHeaderFilePath << std::endl;
                return FAIL;
            }
            metaHeaderFile << "#pragma once" << std::endl
                           << "#include <cstdint>" << std::endl
                           << std::endl;
            std::string varShaderMetaName = shader_meta_name(inputFileBaseName);
            WriteByteArray(metaHeaderFile, metaJsonContent.data(), metaJsonContent.size(), varShaderMetaName);
            std::cout << "shader meta - header file saved in: " << metaHeaderFilePath << std::endl;
            
            // generate tag file for incremental build
            std::string tagFilePath = SEEK_GENERATED_TAG_DIR "/" SHADER_COMPILE_FILE_SUFFIX "/" + inputFileBaseName + SHADER_TAG_FILE_SUFFIX;
            int generateRet = GenerateTagFile(tagFilePath);
            if (generateRet != SUCCESS)
                return generateRet;
            std::cout << "tag file saved in: " << tagFilePath << std::endl;
        }
        else
        {
            sourceDesc.loadIncludeCallback = includeFileHandleFunc;
            int success = iterateFunc(extPredefines.begin(), extPredefines.end(), extPredefines.begin(), fixed_macros, extPredefines, preprocessFunc);
            if (success != SUCCESS)
                return success;

            bool needToGenerateDependFile = false;
            std::string dependFilePath = SEEK_GENERATED_DEPEND_DIR "/" + inputFileBaseName + SHADER_DEPEND_FILE_SUFFIX;
            std::string oldDependFileContent;
            if (SUCCESS != read_file_content(dependFilePath, oldDependFileContent))
            {
                needToGenerateDependFile = true;
                std::cerr << "no depend file, generate a new depend file" << std::endl;
                std::cerr << "new dependency:" << std::endl;
                for (auto& _new : dependIncludeFiles)
                {
                    std::cerr << "  " << _new.first << "=" << _new.second << std::endl;
                }
            }
            else
            {
                std::stringstream sstream;
                sstream << oldDependFileContent;
                std::string dependName;
                std::map<std::string, std::string> oldDependIncludeFiles;
                while (std::getline(sstream, dependName, ';'))
                {
                    size_t pos = dependName.find('=');
                    std::string _name = dependName.substr(0, pos);
                    std::string _hash = dependName.substr(pos + 1);
                    oldDependIncludeFiles[_name] = _hash;
                }
                if (dependIncludeFiles != oldDependIncludeFiles)
                {
                    needToGenerateDependFile = true;
                    std::cerr << "dependent file is changed, regenerate depend file" << std::endl;
                    std::cerr << "old dependency:" << std::endl;
                    for (auto& _old : oldDependIncludeFiles)
                    {
                        std::cerr << "  " << _old.first << "=" << _old.second << std::endl;
                    }
                    std::cerr << "new dependency:" << std::endl;
                    for (auto& _new : dependIncludeFiles)
                    {
                        std::cerr << _new.first << "=" << _new.second << std::endl;
                    }
                }
            }

            if (needToGenerateDependFile)
            {
                std::ofstream dependFile(dependFilePath, std::ios_base::binary);
                if (!dependFile)
                {
                    std::cerr << "couldn't open the depend file: " << dependFilePath << std::endl;
                    return FAIL;
                }

                for (auto it = dependIncludeFiles.begin(); it != dependIncludeFiles.end(); it++)
                {
                    if (it != dependIncludeFiles.begin())
                        dependFile << ";";
                    dependFile << it->first << "=" << it->second;
                }

                std::cout << "shader depend file: " << dependFilePath << std::endl;
            }
            else
            {
                std::cerr << "no need to generate depend file" << std::endl;
            }

            std::string tagFilePath = SEEK_GENERATED_TAG_DIR "/" SHADER_DEPEND_FILE_SUFFIX "/" + inputFileBaseName + SHADER_TAG_FILE_SUFFIX;
            int generateRet = GenerateTagFile(tagFilePath);
            if (generateRet != SUCCESS)
                return generateRet;
            std::cout << "tag file saved in: " << tagFilePath << std::endl;
        }

        return SUCCESS;
    }
    catch (std::exception& ex)
    {
        std::cerr << "exception happened: " << ex.what() << std::endl;
        return FAIL;
    }

    return SUCCESS;
}
