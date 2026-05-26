#include "seek.config.h"
#include "shader_helper.h"
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#if defined(_WIN32)
#include <windows.h>
#endif
#include "CLI/CLI.hpp"
#include "shader_content_def.h"
#include <slang.h>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>

using namespace shadercompiler;

// Slang COM pointer aliases
using SlangGlobalSessionPtr   = Slang::ComPtr<slang::IGlobalSession>;
using SlangSessionPtr         = Slang::ComPtr<slang::ISession>;
using SlangModulePtr          = Slang::ComPtr<slang::IModule>;
using SlangEntryPointPtr      = Slang::ComPtr<slang::IEntryPoint>;
using SlangComponentTypePtr   = Slang::ComPtr<slang::IComponentType>;
using SlangBlobPtr            = Slang::ComPtr<slang::IBlob>;

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
    else if (stageName == "gs")
    {
        return ShaderStage::GeometryShader;
    }
    else if (stageName == "hs")
    {
        return ShaderStage::HullShader;
    }
    else if (stageName == "ds")
    {
        return ShaderStage::DomainShader;
    }
    else if (stageName == "cs")
    {
        return ShaderStage::ComputeShader;
    }
    else
    {
        return ShaderStage::NumShaderStages;
    }
}

static inline SlangStage SlangStageFromShaderStage(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::VertexShader:   return SLANG_STAGE_VERTEX;
        case ShaderStage::PixelShader:    return SLANG_STAGE_FRAGMENT;
        case ShaderStage::GeometryShader: return SLANG_STAGE_GEOMETRY;
        case ShaderStage::HullShader:     return SLANG_STAGE_HULL;
        case ShaderStage::DomainShader:   return SLANG_STAGE_DOMAIN;
        case ShaderStage::ComputeShader:  return SLANG_STAGE_COMPUTE;
        default:                          return SLANG_STAGE_NONE;
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

static inline SlangCompileTarget ParseSlangCodeGenTarget(ShadingLanguage language)
{
    switch (language)
    {
        case ShadingLanguage::Dxil:      return SLANG_DXIL;
        case ShadingLanguage::SpirV:     return SLANG_SPIRV;
        case ShadingLanguage::Hlsl:      return SLANG_HLSL;
        case ShadingLanguage::Glsl:      return SLANG_GLSL;
        case ShadingLanguage::Essl:      return SLANG_GLSL;
        case ShadingLanguage::Msl_macOS: return SLANG_METAL;
        case ShadingLanguage::Msl_iOS:   return SLANG_METAL;
        default:                         return SLANG_HLSL;
    }
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

static void ParseSlangReflection(slang::IComponentType* program, const std::string& stageName, ReflectInfo& reflectInfo)
{
    reflectInfo.stage = stageName;
    reflectInfo.entry_point = "main";
    reflectInfo.code_type = CodeType::SourceCode;

    if (!program) return;

    slang::ProgramLayout* layout = program->getLayout();
    if (!layout) return;

    // --- Resources ---
    SlangInt paramCount = layout->getParameterCount();
    for (SlangInt i = 0; i < paramCount; i++)
    {
        slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
        if (!param) continue;

        slang::ParameterCategory category = param->getCategory();
        ResourceInfo resourceInfo;

        if (category == slang::ParameterCategory::ConstantBuffer ||
            category == slang::ParameterCategory::Uniform)
        {
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
            resourceInfo.type = ResourceType::ConstantBuffer;
            resourceInfo.binding = static_cast<uint32_t>(param->getBindingIndex());
            resourceInfo.bindCount = 1;
            const char* name = param->getName();
            resourceInfo.name = name ? name : "";
            resourceInfo.size = typeLayout ? static_cast<uint32_t>(typeLayout->getSize()) : 0;
        }
        else if (category == slang::ParameterCategory::ShaderResource)
        {
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
            slang::TypeReflection* type = typeLayout ? typeLayout->getType() : nullptr;
            if (type)
            {
                SlangResourceShape shape = type->getResourceShape();
                resourceInfo.type = (shape == SLANG_RESOURCE_NONE) ? ResourceType::Buffer : ResourceType::Texture;
            }
            else
            {
                resourceInfo.type = ResourceType::Texture;
            }
            resourceInfo.binding = static_cast<uint32_t>(param->getBindingIndex());
            resourceInfo.bindCount = 1;
            const char* name = param->getName();
            resourceInfo.name = name ? name : "";
            resourceInfo.size = 0;
        }
        else if (category == slang::ParameterCategory::UnorderedAccess)
        {
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
            slang::TypeReflection* type = typeLayout ? typeLayout->getType() : nullptr;
            if (type)
            {
                SlangResourceShape shape = type->getResourceShape();
                resourceInfo.type = (shape == SLANG_RESOURCE_NONE) ? ResourceType::RWBuffer : ResourceType::RWTexture;
            }
            else
            {
                resourceInfo.type = ResourceType::RWBuffer;
            }
            resourceInfo.binding = static_cast<uint32_t>(param->getBindingIndex());
            resourceInfo.bindCount = 1;
            const char* name = param->getName();
            resourceInfo.name = name ? name : "";
            resourceInfo.size = 0;
        }
        else if (category == slang::ParameterCategory::SamplerState)
        {
            resourceInfo.type = ResourceType::Sampler;
            resourceInfo.binding = static_cast<uint32_t>(param->getBindingIndex());
            resourceInfo.bindCount = 1;
            const char* name = param->getName();
            resourceInfo.name = name ? name : "";
            resourceInfo.size = 0;
        }
        else
        {
            continue;
        }

        reflectInfo.resources.push_back(resourceInfo);
    }

    // --- Compute shader: thread group size ---
    if (stageName == "cs")
    {
        slang::EntryPointReflection* ep = layout->getEntryPointByIndex(0);
        if (ep)
        {
            SlangUInt sizeAlongAxis[3] = {1, 1, 1};
            ep->getComputeThreadGroupSize(3, sizeAlongAxis);
            reflectInfo.block_size.x = static_cast<uint32_t>(sizeAlongAxis[0]);
            reflectInfo.block_size.y = static_cast<uint32_t>(sizeAlongAxis[1]);
            reflectInfo.block_size.z = static_cast<uint32_t>(sizeAlongAxis[2]);
        }
    }

    // --- Specialization Constants ---
    for (SlangInt i = 0; i < paramCount; i++)
    {
        slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
        if (!param) continue;

        slang::ParameterCategory category = param->getCategory();
        if (category != slang::ParameterCategory::SpecializationConstant)
            continue;

        SpecializationConstantInfo specInfo;
        const char* name = param->getName();
        specInfo.name = name ? name : "";

        // Determine type from Slang type reflection
        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        if (typeLayout)
        {
            slang::TypeReflection* specType = typeLayout->getType();
            if (specType)
            {
                switch (specType->getKind())
                {
                case slang::TypeReflection::Kind::Scalar:
                {
                    switch (specType->getScalarType())
                    {
                    case SLANG_SCALAR_TYPE_INT32:   specInfo.typeName = "int";   specInfo.defaultValue = "0"; break;
                    case SLANG_SCALAR_TYPE_FLOAT32: specInfo.typeName = "float"; specInfo.defaultValue = "0.0"; break;
                    case SLANG_SCALAR_TYPE_BOOL:    specInfo.typeName = "bool";  specInfo.defaultValue = "false"; break;
                    default: specInfo.typeName = "unknown"; specInfo.defaultValue = "0"; break;
                    }
                    break;
                }
                default:
                    specInfo.typeName = "unknown";
                    specInfo.defaultValue = "0";
                    break;
                }
            }
            else
            {
                specInfo.typeName = "unknown";
                specInfo.defaultValue = "0";
            }
        }
        else
        {
            specInfo.typeName = "unknown";
            specInfo.defaultValue = "0";
        }
        reflectInfo.specializations.push_back(specInfo);
    }
}

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

// ============================================================================
// Slang Compiler Singleton
// ============================================================================
struct SlangCompiler
{
    SlangGlobalSessionPtr globalSession;
    SlangSessionPtr       session;
    SlangCompileTarget    currentTarget = SLANG_TARGET_UNKNOWN;

    SlangCompiler()
    {
        slang::IGlobalSession* gs = nullptr;
        slang::createGlobalSession(&gs);
        globalSession = gs;

        // Create with no targets initially; target will be set on demand
        slang::SessionDesc sessionDesc = {};
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

        slang::ISession* s = nullptr;
        globalSession->createSession(sessionDesc, &s);
        session = s;
    }

    void ensureTarget(SlangCompileTarget target)
    {
        if (currentTarget == target) return;

        slang::TargetDesc targetDesc = {};
        targetDesc.structureSize = sizeof(slang::TargetDesc);
        targetDesc.format = target;
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        slang::ISession* s = nullptr;
        globalSession->createSession(sessionDesc, &s);
        session = s;
        currentTarget = target;
    }

    ~SlangCompiler()
    {
        session = nullptr;
        globalSession = nullptr;
    }
};

static SlangCompiler* g_slangCompiler = nullptr;

static SlangCompiler& GetSlangCompiler()
{
    if (!g_slangCompiler)
        g_slangCompiler = new SlangCompiler();
    return *g_slangCompiler;
}

static void CleanupSlangCompiler()
{
    delete g_slangCompiler;
    g_slangCompiler = nullptr;
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
        ->check(CLI::IsMember({ "dxil", "spirv", "hlsl", "essl", "msl_macos", "msl_ios"}));

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

        std::string source;
        {
            std::ifstream inputFile(inputFilePath, std::ios_base::binary);
            if (!inputFile)
            {
                std::cerr << "couldn't open the input file: " << inputFilePath << std::endl;
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

        {
            ShaderStage stage = ParseShaderStage(stageName);
            if (stage == ShaderStage::NumShaderStages)
            {
                std::cerr << "invalid shader stage: " << stageName << std::endl;
                return FAIL;
            }
        }

        std::map<std::string, std::string> dependIncludeFiles;

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

        // Target description
        struct TargetDesc { ShadingLanguage language; };
        std::vector<TargetDesc> targetDesc;
        targetDesc.resize(targetName.size());
        for (size_t outputIdx = 0; outputIdx != targetName.size(); outputIdx++)
        {
            targetDesc[outputIdx].language = ParseShadingLanguage(targetName[outputIdx]);
            if (targetDesc[outputIdx].language == ShadingLanguage::NumShadingLanguages)
            {
                std::cerr << "invalid target shading language: " << targetName[outputIdx] << std::endl;
                return FAIL;
            }
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
                macroDefines.emplace_back(MacroDefine{ macro.name.c_str(), macro.value.c_str() });
            std::map<std::string, std::string> activeMacroMap;
            for (auto& macro : active_macros)
            {
                macroDefines.emplace_back(MacroDefine{ macro.name.c_str(), macro.value.c_str() });
                activeMacroMap[macro.name] = macro.value;
            }

            size_t seed = 0;
            {
                for (auto mapIt : activeMacroMap)
                    HashRange(seed, mapIt.second.begin(), mapIt.second.end());
            }

            SlangCompiler& slang = GetSlangCompiler();

            // Build source with macros as prelude
            std::string preamble;
            for (auto& m : macroDefines)
            {
                preamble += "#define " + std::string(m.name) + " " + std::string(m.value) + "\n";
            }
            std::string fullSource = preamble + source;

            // Step 1: Load module
            SlangModulePtr module;
            {
                SlangBlobPtr diagBlob;
                slang::IBlob* diag = nullptr;
                slang::IModule* m = slang.session->loadModuleFromSourceString(
                    inputFileBaseName.c_str(),
                    inputFilePath.c_str(),
                    fullSource.c_str(),
                    &diag);
                if (diag && diag->getBufferSize() > 0)
                {
                    const char* msg = static_cast<const char*>(diag->getBufferPointer());
                    std::cerr << "Slang diagnostics for " << inputFileBaseName << ": "
                              << std::string(msg, msg + diag->getBufferSize()) << std::endl;
                }
                if (!m)
                {
                    std::cerr << "Slang: loadModuleFromSource failed for " << inputFileBaseName << std::endl;
                    return FAIL;
                }
                module = m;
            }

            // Step 2: Find entry point
            SlangEntryPointPtr entryPoint;
            {
                ShaderStage shaderStage = ParseShaderStage(_shaderMetaInfo.stage);
                SlangStage slangStage = SlangStageFromShaderStage(shaderStage);
                slang::IEntryPoint* ep = nullptr;
                SlangResult sr = module->findAndCheckEntryPoint("main", slangStage, &ep, nullptr);
                if (SLANG_FAILED(sr) || !ep)
                {
                    std::cerr << "Slang: findAndCheckEntryPoint('main') failed for " << inputFileBaseName << std::endl;
                    return FAIL;
                }
                entryPoint = ep;
            }

            // Step 3: Ensure session has the target configured
            SlangCompileTarget slangTarget = ParseSlangCodeGenTarget(targetDesc[0].language);
            slang.ensureTarget(slangTarget);

            // Step 4: Create composite component type
            SlangComponentTypePtr program;
            {
                slang::IComponentType* components[] = { module.get(), entryPoint.get() };
                slang::IComponentType* prog = nullptr;
                SlangResult sr = slang.session->createCompositeComponentType(
                    components, 2,
                    &prog);
                if (SLANG_FAILED(sr) || !prog)
                {
                    std::cerr << "Slang: createCompositeComponentType failed for " << inputFileBaseName << std::endl;
                    return FAIL;
                }
                program = prog;
            }

            int compileRet = SUCCESS;
            for (size_t resultIdx = 0; resultIdx != targetDesc.size(); resultIdx++)
            {
                SlangBlobPtr codeBlob;
                SlangBlobPtr diagBlob;
                {
                    slang::IBlob* code = nullptr;
                    slang::IBlob* diag = nullptr;
                    SlangResult sr = program->getTargetCode(
                        static_cast<SlangInt>(resultIdx), &code, &diag);
                    codeBlob = code;
                    diagBlob = diag;
                    if (SLANG_FAILED(sr))
                    {
                        std::cerr << "Slang: getTargetCode failed for target " << resultIdx << std::endl;
                        if (diagBlob && diagBlob->getBufferSize() > 0)
                        {
                            const char* msg = static_cast<const char*>(diagBlob->getBufferPointer());
                            std::cerr << "Compile diagnostics: " << std::string(msg, msg + diagBlob->getBufferSize()) << std::endl;
                        }
                        compileRet = FAIL;
                        continue;
                    }
                }

                if (diagBlob && diagBlob->getBufferSize() > 0)
                {
                    const char* msg = static_cast<const char*>(diagBlob->getBufferPointer());
                    std::cerr << "Slang diagnostics: " << std::string(msg, msg + diagBlob->getBufferSize()) << std::endl;
                }

                if (!codeBlob || codeBlob->getBufferSize() == 0)
                {
                    std::cerr << "Slang: empty output for target " << resultIdx << std::endl;
                    compileRet = FAIL;
                    continue;
                }

                const void* shaderSourceData = codeBlob->getBufferPointer();
                uint32_t   shaderSourceSize  = static_cast<uint32_t>(codeBlob->getBufferSize());

                const void* shaderHeaderData = shaderSourceData;
                uint32_t   shaderHeaderSize  = shaderSourceSize;
                std::string byteCodeContent;
                if (generateByteCode)
                {
                    shaderHeaderData = shaderSourceData;
                    shaderHeaderSize = shaderSourceSize;
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

                // Shader code source file
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

                // Shader code header file
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

                // Shader reflect
                std::string outReflectSourceFilePath = outputFileDir + "/" + outputFileName + SHADER_REFLECT_FILE_SUFFIX;
                ReflectInfo reflectInfo;
                ParseSlangReflection(program.get(), stageName, reflectInfo);
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
            // Build preamble with macros
            std::string preamble;
            for (auto& macro : fixed_macros)
                preamble += "#define " + std::string(macro.name) + " " + std::string(macro.value) + "\n";
            for (auto& macro : active_macros)
                preamble += "#define " + std::string(macro.name) + " " + std::string(macro.value) + "\n";

            SlangCompiler& slang = GetSlangCompiler();
            SlangModulePtr module;
            {
                std::string fullSource = preamble + source;
                slang::IModule* m = slang.session->loadModuleFromSourceString(
                    inputFileBaseName.c_str(),
                    inputFilePath.c_str(),
                    fullSource.c_str());
                module = m;
            }

            if (module)
            {
                SlangInt depCount = module->getDependencyFileCount();
                for (SlangInt i = 0; i < depCount; i++)
                {
                    const char* depPath = module->getDependencyFilePath(i);
                    if (depPath)
                    {
                        std::string depName(depPath);
                        // Normalize to relative path
                        std::string shaderSourceDir = SEEK_SHADER_SOURCE_DIR;
                        if (depName.find(shaderSourceDir + "/") == 0)
                            depName = depName.substr(shaderSourceDir.size() + 1);
                        else if (depName.find(shaderSourceDir + "\\") == 0)
                            depName = depName.substr(shaderSourceDir.size() + 1);

                        size_t hash = std::hash<std::string>{}(depName);
                        dependIncludeFiles[depName] = std::to_string(hash);
                    }
                }
            }

            return SUCCESS;
        };

        // compile and preprocess is mutual exclusion
        if (!generateDependFile)
        {
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

        CleanupSlangCompiler();
        return SUCCESS;
    }
    catch (std::exception& ex)
    {
        std::cerr << "exception happened: " << ex.what() << std::endl;
        return FAIL;
    }

    CleanupSlangCompiler();
    return SUCCESS;
}
