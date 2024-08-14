#pragma once
#include <map>
#include <string>

using ShaderContentMap = std::map<std::string, std::pair<const void*, size_t>>;
using ShaderContent = ShaderContentMap::mapped_type;

//#define ___SHADER_META      __g_shader_meta_
//#define ___SHADER_CODE      __g_shader_code_
//#define ___SHADER_REFLECT   __g_shader_reflect_

#define SHADER_META_NAME(shader)                        __g_shader_meta_##shader
#define SHADER_CODE_NAME(shader, language, debug)       __g_shader_code_##language##_##shader##debug
#define SHADER_REFLECT_NAME(shader, language, debug)    __g_shader_reflect_##language##_##shader##debug

inline std::string shader_meta_name(const std::string& shader)
{
    return "__g_shader_meta_" + shader;
}

inline std::string shader_code_name(const std::string& shader, const std::string& language, const std::string& debug)
{
    return "__g_shader_code_" + language + "_" + shader + debug;
}

inline std::string shader_reflect_name(const std::string& shader, const std::string& language, const std::string& debug)
{
    return "__g_shader_reflect_" + language + "_" + shader + debug;
}

#define SHADER_META_MAP_MEMBER(shader) \
    { #shader , { SHADER_META_NAME(shader), sizeof(SHADER_META_NAME(shader)) } },

#define SHADER_CODE_MAP_MEMBER(shader, language, debug) \
    { #shader , { SHADER_CODE_NAME(shader, language, debug), sizeof(SHADER_CODE_NAME(shader, language, debug)) } },

#define SHADER_CODE_MAP_MEMBER_VAR(shader, var) \
    { #shader , { var, sizeof(var) } },

#define SHADER_REFLECT_MAP_MEMBER(shader, language, debug) \
    { #shader , { SHADER_REFLECT_NAME(shader, language, debug), sizeof(SHADER_REFLECT_NAME(shader, language, debug)) } },

#define SHADER_REFLECT_MAP_MEMBER_VAR(shader, var) \
    { #shader , { var, sizeof(var) } },

#define SHADER_CODE_MAP_GROUP(shader, language, debug)    SHADER_CODE_MAP_GROUP_##language##_##shader##debug
#define SHADER_REFLECT_MAP_GROUP(shader, language, debug) SHADER_REFLECT_MAP_GROUP_##language##_##shader##debug

#define SHADER_META_MAP_NAME()                   __g_shader_meta_map
#define SHADER_CODE_MAP_NAME(language, debug)    __g_shader_code_map_##language##debug
#define SHADER_REFLECT_MAP_NAME(language, debug) __g_shader_reflect_map_##language##debug

#define FUNC_QueryShaderCodeContent(language, debug) \
    inline ShaderContent QueryShaderCodeContent_##language##debug(const std::string& shaderName) \
    { \
        auto it = SHADER_CODE_MAP_NAME(language, debug).find(shaderName); \
        if (it == SHADER_CODE_MAP_NAME(language, debug).end()) \
            return ShaderContent{ nullptr, 0 }; \
        return it->second; \
    }

#define FUNC_QueryShaderReflectContent(language, debug) \
    inline ShaderContent QueryShaderReflectContent_##language##debug(const std::string& shaderName) \
    { \
        auto it = SHADER_REFLECT_MAP_NAME(language, debug).find(shaderName); \
        if (it == SHADER_REFLECT_MAP_NAME(language, debug).end()) \
            return ShaderContent{ nullptr, 0 }; \
        return it->second; \
    }

#define FUNC_QueryShaderMetaContent() \
    inline ShaderContent QueryShaderMetaContent(const std::string& shaderName) \
    { \
        auto it = SHADER_META_MAP_NAME().find(shaderName); \
        if (it == SHADER_META_MAP_NAME().end()) \
            return ShaderContent{ nullptr, 0 }; \
        return it->second; \
    }

#define QueryShaderCodeContent_Member(language, debug) \
    if (shaderLanguage == #language) \
        return QueryShaderCodeContent_##language##debug(shaderName);

#define QueryShaderReflectContent_Member(language, debug) \
    if (shaderLanguage == #language) \
        return QueryShaderReflectContent_##language##debug(shaderName);
