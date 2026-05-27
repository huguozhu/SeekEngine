if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.27")
    cmake_policy(SET CMP0147 NEW) # Visual Studio Generators build custom commands in parallel.
endif()

set(shader_shared_files
    ${SEEK_SHADER_SOURCE_DIR}/shared/Blur.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/ColorReplace.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/CommonTypes.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/GI.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/LiquidGlass.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/MetaBall.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleCommonTypes.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/SkyBox.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/Sprite2D.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/Ssao.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/Taa.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/WaterMark.slang
    ${SEEK_SHADER_SOURCE_DIR}/shared/Waves.slang
)

set(shader_header_files
    ${SEEK_SHADER_SOURCE_DIR}/BRDF.slangh
    ${SEEK_SHADER_SOURCE_DIR}/CalcSDF.slangh
    ${SEEK_SHADER_SOURCE_DIR}/Common.slangh
    ${SEEK_SHADER_SOURCE_DIR}/IBL.slangh
    ${SEEK_SHADER_SOURCE_DIR}/Lighting.slangh
    ${SEEK_SHADER_SOURCE_DIR}/MorphTarget.slangh
    ${SEEK_SHADER_SOURCE_DIR}/Skeleton.slangh
    ${SEEK_SHADER_SOURCE_DIR}/ToneMapping.slangh
    ${SEEK_SHADER_SOURCE_DIR}/CommonCBuffer.slangh
    ${SEEK_SHADER_SOURCE_DIR}/VertexCommon.slangh
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleCommon.slangh
    ${SEEK_SHADER_SOURCE_DIR}/Shadow.slangh
)

set(shader_sources_files
    ${SEEK_SHADER_SOURCE_DIR}/BlurPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/CopyPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/CubeMapVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/DeferredLightingVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/DeferredLightingPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/EmptyPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/EquirectangularToCubeMapPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ForwardRenderingCommonPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/FxaaPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GenBRDF2DLutPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GenerateCascadedShadowMapPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GenerateCubeShadowMapPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GenerateGBufferPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GenerateRsmPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GiRsmPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GiLpvPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/GiVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/IrradianceConvolutionPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/LightCullingCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/LiquidGlassPS.slang    
    ${SEEK_SHADER_SOURCE_DIR}/LPVInjectPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/LPVInjectVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/LPVPropagationPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/LPVPropagationVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/MeshRenderingVS.slang

    ${SEEK_SHADER_SOURCE_DIR}/PostProcessVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/PrefilterEnvPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/PreZMeshRenderingVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Sprite2DMetaballPS.slang    
   
    ${SEEK_SHADER_SOURCE_DIR}/ShadowCopyPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ShadowingDirectionalPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ShadowingPointPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ShadowingSpotPS.slang
    
    ${SEEK_SHADER_SOURCE_DIR}/Sprite2DVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/SsaoVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/SsaoPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/TaaPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ToneMappingPS.slang

    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/WaterMarkGenerateCS.slang

    # Particle shaders
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleInitCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleTickBeginCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleEmitCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleSimulateCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleCullingCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticlePreSortCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleBitonicSortCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleSortMatrixTransposeCS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleRenderVS.slang
    ${SEEK_SHADER_SOURCE_DIR}/Particles/ParticleRenderPS.slang
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
    file(RELATIVE_PATH single_shader_relpath "${SEEK_SHADER_SOURCE_DIR}" "${single_shader_sources_file}")

    set(single_tag_depend_tagfile "${generated_shader_tag_depend_dir}/${single_shader_basename}${SHADER_TAG_FILE_SUFFIX}")
    set(single_depend_dependfile  "${generated_shader_depend_dir}/${single_shader_basename}${SHADER_DEPEND_FILE_SUFFIX}")
    add_custom_command(
        OUTPUT ${single_tag_depend_tagfile} ${single_depend_dependfile}
        COMMAND ${ShaderCompiler_bin} --input "${single_shader_relpath}" --depend
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
elseif (shader_target_platform STREQUAL "dxil")
    list(APPEND shader_compile_options "--define=\"SEEK_DXIL=1\"")  
elseif (shader_target_platform STREQUAL "spirv")
    list(APPEND shader_compile_options "--define=\"SEEK_SPIRV=1\"")
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
    file(RELATIVE_PATH single_shader_relpath "${SEEK_SHADER_SOURCE_DIR}" "${single_shader_sources_file}")

    set(single_depend_dependfile  "${generated_shader_depend_dir}/${single_shader_basename}${SHADER_DEPEND_FILE_SUFFIX}")
    set(single_tag_compile_tagfile "${generated_shader_tag_compile_dir}/${single_shader_basename}${SHADER_TAG_FILE_SUFFIX}")

    add_custom_command(
        OUTPUT ${single_tag_compile_tagfile}
        COMMAND ${ShaderCompiler_bin} --input=${single_shader_relpath} --target=${shader_target_platform} ${shader_compile_options}
        DEPENDS ${single_shader_sources_file} ${single_depend_dependfile} # also depend on ${ShaderCompiler_bin}, but ShaderCompiler always been build which make incremental build no meaning
        WORKING_DIRECTORY ${ShaderCompiler_working_dir})

    list(APPEND all_generated_tag_compile_tagfiles ${single_tag_compile_tagfile})
endforeach()

############################## 多入口点着色器 ##############################
# 格式：每行 "源文件|输出名|stage|入口函数"
# 同一源文件可有多行，分别编译为不同的输出名（如将 VS 和 PS 合并到一个 .slang 文件）
set(multi_entry_shaders
    # SkyBox VS + PS 合并到一个 .slang 文件
    "SkyBoxRendering.slang|SkyBox_VS|vs|skybox_vs"
    "SkyBoxRendering.slang|SkyBox_PS|ps|skybox_ps"
)

set(multi_entry_output_names "")
set(multi_depend_sources_processed "")

foreach(multi_entry ${multi_entry_shaders})
    string(REPLACE "|" ";" multi_parts "${multi_entry}")
    list(GET multi_parts 0 multi_source)
    list(GET multi_parts 1 multi_output)
    list(GET multi_parts 2 multi_stage)
    list(GET multi_parts 3 multi_entry_name)

    get_filename_component(multi_source_basename "${multi_source}" NAME_WLE)

    # 依赖分析：每个源文件只需生成一次
    list(FIND multi_depend_sources_processed "${multi_source}" depend_done)
    if(depend_done EQUAL -1)
        set(single_tag_depend_tagfile "${generated_shader_tag_depend_dir}/${multi_source_basename}${SHADER_TAG_FILE_SUFFIX}")
        set(single_depend_dependfile  "${generated_shader_depend_dir}/${multi_source_basename}${SHADER_DEPEND_FILE_SUFFIX}")
        add_custom_command(
            OUTPUT ${single_tag_depend_tagfile} ${single_depend_dependfile}
            COMMAND ${ShaderCompiler_bin} --input "${multi_source}" --depend
            DEPENDS "${SEEK_SHADER_SOURCE_DIR}/${multi_source}" ${shader_header_files} ${shader_shared_files}
            WORKING_DIRECTORY ${ShaderCompiler_working_dir}
        )
        list(APPEND all_generated_tag_depend_tagfiles ${single_tag_depend_tagfile})
        list(APPEND all_generated_depend_dependfiles ${single_depend_dependfile})
        list(APPEND multi_depend_sources_processed "${multi_source}")
    endif()

    set(single_depend_dependfile "${generated_shader_depend_dir}/${multi_source_basename}${SHADER_DEPEND_FILE_SUFFIX}")

    # 编译
    set(single_tag_compile_tagfile "${generated_shader_tag_compile_dir}/${multi_output}${SHADER_TAG_FILE_SUFFIX}")
    add_custom_command(
        OUTPUT ${single_tag_compile_tagfile}
        COMMAND ${ShaderCompiler_bin} --input="${multi_source}" --output="${multi_output}" --stage="${multi_stage}" --entry="${multi_entry_name}" --target=${shader_target_platform} ${shader_compile_options}
        DEPENDS "${SEEK_SHADER_SOURCE_DIR}/${multi_source}" ${single_depend_dependfile}
        WORKING_DIRECTORY ${ShaderCompiler_working_dir}
    )
    list(APPEND all_generated_tag_compile_tagfiles ${single_tag_compile_tagfile})
    list(APPEND multi_entry_output_names ${multi_output})
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
