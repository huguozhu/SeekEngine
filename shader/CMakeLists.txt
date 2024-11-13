if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.27")
    cmake_policy(SET CMP0147 NEW) # Visual Studio Generators build custom commands in parallel.
endif()

set(shader_shared_files
    ${SEEK_SHADER_SOURCE_DIR}/shared/ColorReplace.h
    ${SEEK_SHADER_SOURCE_DIR}/shared/common.h
    ${SEEK_SHADER_SOURCE_DIR}/shared/ParticleCommon.h
    ${SEEK_SHADER_SOURCE_DIR}/shared/SkyBox.h
    ${SEEK_SHADER_SOURCE_DIR}/shared/WaterMark.h
)

set(shader_header_files
    ${SEEK_SHADER_SOURCE_DIR}/BRDF.dsh
    ${SEEK_SHADER_SOURCE_DIR}/Common.dsh
    ${SEEK_SHADER_SOURCE_DIR}/ColorSpaceCvt_common.dsh
    ${SEEK_SHADER_SOURCE_DIR}/Lighting.dsh
    ${SEEK_SHADER_SOURCE_DIR}/MorphTarget.dsh
    ${SEEK_SHADER_SOURCE_DIR}/Skeleton.dsh
    ${SEEK_SHADER_SOURCE_DIR}/ToneMapping.dsh
    ${SEEK_SHADER_SOURCE_DIR}/CommonCBuffer.dsh
    ${SEEK_SHADER_SOURCE_DIR}/VertexCommon.dsh
    ${SEEK_SHADER_SOURCE_DIR}/ParticleCommon.dsh
)

set(shader_sources_files
    ${SEEK_SHADER_SOURCE_DIR}/CopyPS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ForwardRenderingCommonPS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/MeshRenderingVS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/PostProcessVS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/RGBAToAI420CS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ToneMappingPS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/SkyBoxVS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/SkyBoxPS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkPS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkVS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkGenerateCS.dsf

    ${SEEK_SHADER_SOURCE_DIR}/ParticleInitCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleTickBeginCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleEmitCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleSimulateCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleCullingCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticlePreSortCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleBitonicSortCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleSortMatrixTransposeCS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleRenderVS.dsf
    ${SEEK_SHADER_SOURCE_DIR}/ParticleRenderPS.dsf
)

################## shader_target_platform ##################
if(TARGET_PLATFORM_WINDOWS)
    set(shader_target_platform "hlsl")
elseif(TARGET_PLATFORM_MACOS)
    set(shader_target_platform "msl_macos")
elseif(TARGET_PLATFORM_IOS)
    set(shader_target_platform "msl_ios")
elseif(TARGET_PLATFORM_LINUX)
    set(shader_target_platform "essl")
elseif(TARGET_PLATFORM_ANDROID OR TARGET_PLATFORM_WEB)
    set(shader_target_platform "essl")
endif()

message(STATUS "generated target platform: ${shader_target_platform}")

############################## path ##############################
set(ShaderCompiler_working_dir "${SEEK_SHADER_SOURCE_DIR}")
if(NOT DEFINED ShaderCompiler_bin)
    message(FATAL_ERROR "ShaderCompiler_bin not defined")
endif()

set(generated_shader_depend_dir         ${SEEK_GENERATED_DEPEND_DIR})
set(generated_shader_tag_depend_dir     ${SEEK_GENERATED_TAG_DIR}/${SHADER_DEPEND_FILE_SUFFIX})
set(generated_shader_tag_compile_dir    ${SEEK_GENERATED_TAG_DIR}/${SHADER_COMPILE_FILE_SUFFIX})

############################## depend ##############################
set(all_generated_tag_depend_tagfiles)
set(all_generated_depend_dependfiles)
foreach(single_shader_sources_file ${shader_sources_files})
    get_filename_component(single_shader_filename ${single_shader_sources_file} NAME)
    get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)

    set(single_tag_depend_tagfile "${generated_shader_tag_depend_dir}/${single_shader_basename}${SHADER_TAG_FILE_SUFFIX}")
    set(single_depend_dependfile  "${generated_shader_depend_dir}/${single_shader_basename}${SHADER_DEPEND_FILE_SUFFIX}")
    add_custom_command(
        OUTPUT ${single_tag_depend_tagfile} ${single_depend_dependfile}
        COMMAND ${ShaderCompiler_bin} --input "${single_shader_filename}" --depend
        DEPENDS ${single_shader_sources_file} ${shader_header_files} ${shader_shared_files} # also depend on ${ShaderCompiler_bin}, but ShaderCompiler always been build which make incremental build no meaning
        WORKING_DIRECTORY ${ShaderCompiler_working_dir}
    )
    list(APPEND all_generated_tag_depend_tagfiles ${single_tag_depend_tagfile})
    list(APPEND all_generated_depend_dependfiles ${single_depend_dependfile})
endforeach()
############################## compile ##############################
set(shader_compile_options)

if (shader_target_platform STREQUAL "hlsl")
    list(APPEND shader_compile_options "--define=\"SEEK_HLSL=1\"")
elseif(shader_target_platform STREQUAL "essl")
    list(APPEND shader_compile_options "--define=\"SEEK_GLSL_ES=1\"")
elseif(shader_target_platform STREQUAL "msl_macos")
    list(APPEND shader_compile_options "--define=\"SEEK_MSL_MACOS=1\"")
elseif(shader_target_platform STREQUAL "msl_ios")
    list(APPEND shader_compile_options "--define=\"SEEK_MSL_IOS=1\"")
elseif(shader_target_platform STREQUAL "spriv")
    list(APPEND shader_compile_options "--define=\"SEEK_SPRIV=1\"")
endif()

if (${SEEK_SHADER_GENERATE_DEBUG})
    list(APPEND shader_compile_options "--debug")
endif()

if (SEEK_SHADER_GENERATE_BYTE_CODE)
    list(APPEND shader_compile_options "--bytecode")
endif()

set(all_generated_tag_compile_tagfiles)
foreach(single_shader_sources_file ${shader_sources_files})
    get_filename_component(single_shader_filename ${single_shader_sources_file} NAME)
    get_filename_component(single_shader_basename ${single_shader_sources_file} NAME_WLE)

    set(single_depend_dependfile  "${generated_shader_depend_dir}/${single_shader_basename}${SHADER_DEPEND_FILE_SUFFIX}")
    set(single_tag_compile_tagfile "${generated_shader_tag_compile_dir}/${single_shader_basename}${SHADER_TAG_FILE_SUFFIX}")

    add_custom_command(
        OUTPUT ${single_tag_compile_tagfile}
        COMMAND ${ShaderCompiler_bin} --input=${single_shader_filename} --target=${shader_target_platform} ${shader_compile_options}
        DEPENDS ${single_shader_sources_file} ${single_depend_dependfile} # also depend on ${ShaderCompiler_bin}, but ShaderCompiler always been build which make incremental build no meaning
        WORKING_DIRECTORY ${ShaderCompiler_working_dir})

    list(APPEND all_generated_tag_compile_tagfiles ${single_tag_compile_tagfile})
endforeach()

message(STATUS ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/CMakeListst.gnerate_header.cmake)

# interface for engine
add_custom_target(GenerateShaders
    DEPENDS ${all_generated_tag_compile_tagfiles}
    SOURCES ${shader_shared_files} ${shader_header_files} ${shader_sources_files}
)
add_dependencies(GenerateShaders ShaderCompiler)
source_group("Shared" FILES ${shader_shared_files})
source_group("Header" FILES ${shader_header_files})
source_group("Source" FILES ${shader_sources_files})
