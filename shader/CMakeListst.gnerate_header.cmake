
create_directory(${SEEK_GENERATED_DEPEND_DIR})
create_directory(${SEEK_GENERATED_META_DIR})
create_directory(${SEEK_GENERATED_TAG_DIR}/${SHADER_COMPILE_FILE_SUFFIX})
create_directory(${SEEK_GENERATED_TAG_DIR}/${SHADER_DEPEND_FILE_SUFFIX})

# create directory
foreach(TARGET_PLATFORM ${shader_target_platform})
    create_directory(${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM})
    if (SEEK_SHADER_GENERATE_DEBUG)
        create_directory(${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}${SHADER_DEBUG_DIR_SUFFIX})
    endif()
endforeach()

###################################### generate shader content header file ######################################
set(GENERATED_SHADER_CONTENT_FILE           "shader_content.h")
set(GENERATED_SHADER_META_CONTENT_FILE      "shader_meta_content.h")
set(GENERATED_SHADER_CODE_CONTENT_FILE      "shader_code_content.h")
set(GENERATED_SHADER_REFLECT_CONTENT_FILE   "shader_reflect_content.h")

# GENERATED_SHADER_CONTENT_FILE
set(GENERATED_SHADER_CONTENT_PART1 "#include \"${SEEK_GENERATED_META_DIR}/${GENERATED_SHADER_META_CONTENT_FILE}\"\n")
set(GENERATED_SHADER_CONTENT_PART2 "FUNC_QueryShaderMetaContent()\n")
set(GENERATED_SHADER_CONTENT_PART3 "inline ShaderContent QueryShaderCodeContent(const std::string& shaderLanguage, const std::string& shaderName)\n{\n")
if (SEEK_SHADER_GENERATE_DEBUG)
    set(GENERATED_SHADER_CONTENT_PART4 "inline ShaderContent QueryShaderCodeContent${SHADER_DEBUG_DIR_SUFFIX}(const std::string& shaderLanguage, const std::string& shaderName)\n{\n")
endif()
set(GENERATED_SHADER_CONTENT_PART5 "inline ShaderContent QueryShaderReflectContent(const std::string& shaderLanguage, const std::string& shaderName)\n{\n")
if (SEEK_SHADER_GENERATE_DEBUG)
    set(GENERATED_SHADER_CONTENT_PART6 "inline ShaderContent QueryShaderReflectContent${SHADER_DEBUG_DIR_SUFFIX}(const std::string& shaderLanguage, const std::string& shaderName)\n{\n")
endif()
foreach(TARGET_PLATFORM ${shader_target_platform})
    string(APPEND GENERATED_SHADER_CONTENT_PART1 "#include \"${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}/${GENERATED_SHADER_CODE_CONTENT_FILE}\"\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART2 "FUNC_QueryShaderCodeContent(${TARGET_PLATFORM},)\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART1 "#include \"${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}/${GENERATED_SHADER_REFLECT_CONTENT_FILE}\"\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART2 "FUNC_QueryShaderReflectContent(${TARGET_PLATFORM},)\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART3 "    QueryShaderCodeContent_Member(${TARGET_PLATFORM},)\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART5 "    QueryShaderReflectContent_Member(${TARGET_PLATFORM},)\n")
    if (SEEK_SHADER_GENERATE_DEBUG)
        string(APPEND GENERATED_SHADER_CONTENT_PART1 "#include \"${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}${SHADER_DEBUG_DIR_SUFFIX}/${GENERATED_SHADER_CODE_CONTENT_FILE}\"\n")
        string(APPEND GENERATED_SHADER_CONTENT_PART2 "FUNC_QueryShaderCodeContent(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
        string(APPEND GENERATED_SHADER_CONTENT_PART1 "#include \"${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}${SHADER_DEBUG_DIR_SUFFIX}/${GENERATED_SHADER_REFLECT_CONTENT_FILE}\"\n")
        string(APPEND GENERATED_SHADER_CONTENT_PART2 "FUNC_QueryShaderReflectContent(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
        string(APPEND GENERATED_SHADER_CONTENT_PART4 "    QueryShaderCodeContent_Member(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
        string(APPEND GENERATED_SHADER_CONTENT_PART6 "    QueryShaderReflectContent_Member(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
    endif()
endforeach()
string(APPEND GENERATED_SHADER_CONTENT_PART3 "    return ShaderContent{ nullptr, 0 }\;\n}\n")
string(APPEND GENERATED_SHADER_CONTENT_PART5 "    return ShaderContent{ nullptr, 0 }\;\n}\n")
if (SEEK_SHADER_GENERATE_DEBUG)
    string(APPEND GENERATED_SHADER_CONTENT_PART4 "    return ShaderContent{ nullptr, 0 }\;\n}\n")
    string(APPEND GENERATED_SHADER_CONTENT_PART6 "    return ShaderContent{ nullptr, 0 }\;\n}\n")
endif()
file(WRITE ${SEEK_GENERATED_SHADER_DIR}/${GENERATED_SHADER_CONTENT_FILE}
    "#pragma once\n"
    "#include \"shader_content_def.h\"\n"
    ${GENERATED_SHADER_CONTENT_PART1}
    "\n"
    ${GENERATED_SHADER_CONTENT_PART2}
    "\n"
    ${GENERATED_SHADER_CONTENT_PART3}
    ${GENERATED_SHADER_CONTENT_PART4}
    "\n"
    ${GENERATED_SHADER_CONTENT_PART5}
    ${GENERATED_SHADER_CONTENT_PART6}
)

# GENERATED_SHADER_META_CONTENT_FILE
set(GENERATED_SHADER_META_CONTENT_PART1 "")
set(GENERATED_SHADER_META_CONTENT_PART2 "static const ShaderContentMap SHADER_META_MAP_NAME() =\n{\n")
foreach(single_shader_sources_file ${shader_sources_files})
    get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)
    string(APPEND GENERATED_SHADER_META_CONTENT_PART1 "#include \"${single_shader_basename}.h\"\n")
    string(APPEND GENERATED_SHADER_META_CONTENT_PART2 "    SHADER_META_MAP_MEMBER(${single_shader_basename})\n")
endforeach()
string(APPEND GENERATED_SHADER_META_CONTENT_PART2 "}\;\n")
file(WRITE ${SEEK_GENERATED_META_DIR}/${GENERATED_SHADER_META_CONTENT_FILE}
    "#pragma once\n"
    "#include \"shader_content_def.h\"\n"
    ${GENERATED_SHADER_META_CONTENT_PART1}
    "\n"
    ${GENERATED_SHADER_META_CONTENT_PART2}
)

# GENERATED_SHADER_CODE_CONTENT_FILE
foreach(TARGET_PLATFORM ${shader_target_platform})
    set(GENERATED_SHADER_CODE_CONTENT_PART1 "")
    set(GENERATED_SHADER_CODE_CONTENT_PART2 "static const ShaderContentMap SHADER_CODE_MAP_NAME(${TARGET_PLATFORM},) =\n{\n")
    foreach(single_shader_sources_file ${shader_sources_files})
        get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)
        string(APPEND GENERATED_SHADER_CODE_CONTENT_PART1 "#include \"${single_shader_basename}.h\"\n")
        string(APPEND GENERATED_SHADER_CODE_CONTENT_PART2 "    SHADER_CODE_MAP_GROUP(${single_shader_basename},${TARGET_PLATFORM},)\n")
    endforeach()
    string(APPEND GENERATED_SHADER_CODE_CONTENT_PART2 "}\;\n")
    file(WRITE ${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}/${GENERATED_SHADER_CODE_CONTENT_FILE}
        "#pragma once\n"
        "#include \"shader_content_def.h\"\n"
        ${GENERATED_SHADER_CODE_CONTENT_PART1}
        "\n"
        ${GENERATED_SHADER_CODE_CONTENT_PART2}
    )

    if (SEEK_SHADER_GENERATE_DEBUG)
        set(GENERATED_SHADER_CODE_CONTENT_PART1 "")
        set(GENERATED_SHADER_CODE_CONTENT_PART2 "static const ShaderContentMap SHADER_CODE_MAP_NAME(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX}) =\n{\n")
        foreach(single_shader_sources_file ${shader_sources_files})
            get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)
            string(APPEND GENERATED_SHADER_CODE_CONTENT_PART1 "#include \"${single_shader_basename}.h\"\n")
            string(APPEND GENERATED_SHADER_CODE_CONTENT_PART2 "    SHADER_CODE_MAP_GROUP(${single_shader_basename},${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
        endforeach()
        string(APPEND GENERATED_SHADER_CODE_CONTENT_PART2 "}\;\n")
        file(WRITE ${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}${SHADER_DEBUG_DIR_SUFFIX}/${GENERATED_SHADER_CODE_CONTENT_FILE}
            "#pragma once\n"
            "#include \"shader_content_def.h\"\n"
            ${GENERATED_SHADER_CODE_CONTENT_PART1}
            "\n"
            ${GENERATED_SHADER_CODE_CONTENT_PART2}
        )
    endif()
endforeach()

# GENERATED_SHADER_REFLECT_CONTENT_FILE
foreach(TARGET_PLATFORM ${shader_target_platform})
    set(GENERATED_SHADER_REFLECT_CONTENT_PART1 "")
    set(GENERATED_SHADER_REFLECT_CONTENT_PART2 "static const ShaderContentMap SHADER_REFLECT_MAP_NAME(${TARGET_PLATFORM},) =\n{\n")
    foreach(single_shader_sources_file ${shader_sources_files})
        get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)
        string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART1 "#include \"${single_shader_basename}.h\"\n")
        string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART2 "    SHADER_REFLECT_MAP_GROUP(${single_shader_basename},${TARGET_PLATFORM},)\n")
    endforeach()
    string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART2 "}\;\n")
    file(WRITE ${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}/${GENERATED_SHADER_REFLECT_CONTENT_FILE}
        "#pragma once\n"
        "#include \"shader_content_def.h\"\n"
        ${GENERATED_SHADER_REFLECT_CONTENT_PART1}
        "\n"
        ${GENERATED_SHADER_REFLECT_CONTENT_PART2}
    )

    if (SEEK_SHADER_GENERATE_DEBUG)
        set(GENERATED_SHADER_REFLECT_CONTENT_PART1 "")
        set(GENERATED_SHADER_REFLECT_CONTENT_PART2 "static const ShaderContentMap SHADER_REFLECT_MAP_NAME(${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX}) =\n{\n")
        foreach(single_shader_sources_file ${shader_sources_files})
            get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)
            string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART1 "#include \"${single_shader_basename}.h\"\n")
            string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART2 "    SHADER_REFLECT_MAP_GROUP(${single_shader_basename},${TARGET_PLATFORM},${SHADER_DEBUG_DIR_SUFFIX})\n")
        endforeach()
        string(APPEND GENERATED_SHADER_REFLECT_CONTENT_PART2 "}\;\n")
        file(WRITE ${SEEK_GENERATED_SHADER_DIR}/${TARGET_PLATFORM}${SHADER_DEBUG_DIR_SUFFIX}/${GENERATED_SHADER_REFLECT_CONTENT_FILE}
            "#pragma once\n"
            "#include \"shader_content_def.h\"\n"
            ${GENERATED_SHADER_REFLECT_CONTENT_PART1}
            "\n"
            ${GENERATED_SHADER_REFLECT_CONTENT_PART2}
        )
    endif()
endforeach()
###################################### generate shader content header file ######################################
