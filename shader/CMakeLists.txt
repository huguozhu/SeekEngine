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
    ${SEEK_SHADER_SOURCE_DIR}/TaaPS.slang
    ${SEEK_SHADER_SOURCE_DIR}/ToneMappingPS.slang

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
)

################## shader_target_platform ##################
set(shader_target_platform "hlsl")

message(STATUS "generated target platform: ${shader_target_platform}")

############################## path ##############################
set(ShaderCompiler_working_dir "${SEEK_SHADER_SOURCE_DIR}")
if(NOT DEFINED ShaderCompiler_bin)
    message(FATAL_ERROR "ShaderCompiler_bin not defined")
endif()

set(generated_shader_depend_dir         ${SEEK_GENERATED_DEPEND_DIR})
set(generated_shader_tag_depend_dir     ${SEEK_GENERATED_TAG_DIR}/${SHADER_DEPEND_FILE_SUFFIX})
set(generated_shader_tag_compile_dir    ${SEEK_GENERATED_TAG_DIR}/${SHADER_COMPILE_FILE_SUFFIX})

############################## 顺序着色器编译脚本 ##############################
# ShaderCompiler.exe 多实例并行时存在 .tag/.meta 目录写入竞态
# 用单一 CMake 脚本顺序调用，彻底避免并行冲突
set(SHADER_BUILD_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/build_shaders.cmake")
set(SHADER_STAMP_FILE "${SEEK_GENERATED_TAG_DIR}/.shaders_complete${SHADER_TAG_FILE_SUFFIX}")

add_custom_command(
    OUTPUT ${SHADER_STAMP_FILE}
    COMMAND ${CMAKE_COMMAND} -P "${SHADER_BUILD_SCRIPT}" "${ShaderCompiler_bin}" "${shader_target_platform}" "${SEEK_SHADER_SOURCE_DIR}" "${SHADER_STAMP_FILE}"
    DEPENDS ${shader_sources_files} ${shader_header_files} ${shader_shared_files}
    WORKING_DIRECTORY ${ShaderCompiler_working_dir}
)

# 多入口点着色器输出名列表（供 generate_header.cmake 使用）
set(multi_entry_output_names
    SkyBox_VS SkyBox_PS Ssao_VS Ssao_PS
    LPV_Inject_VS LPV_Inject_PS LPV_Propagation_VS LPV_Propagation_PS
    WaterMark_VS WaterMark_PS ParticleRender_VS ParticleRender_PS
)

message(STATUS ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/CMakeListst.gnerate_header.cmake)

# interface for engine
add_custom_target(GenerateShaders
    DEPENDS ${SHADER_STAMP_FILE}
    SOURCES ${shader_shared_files} ${shader_header_files} ${shader_sources_files}
)
add_dependencies(GenerateShaders ShaderCompiler)
source_group("Shared" FILES ${shader_shared_files})
source_group("Header" FILES ${shader_header_files})
source_group("Source" FILES ${shader_sources_files})
