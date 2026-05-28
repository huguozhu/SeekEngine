# 顺序编译所有着色器，避免 MSBuild 并行批处理导致的 ShaderCompiler 多实例文件竞态
set(COMPILER "${CMAKE_ARGV3}")
set(TARGET "${CMAKE_ARGV4}")
set(SOURCE_DIR "${CMAKE_ARGV5}")

# 多入口点着色器
set(MULTI_ENTRY_SHADERS
    "SkyBoxRendering.slang|SkyBox_VS|skybox_vs|vs"
    "SkyBoxRendering.slang|SkyBox_PS|skybox_ps|ps"
    "SsaoRendering.slang|Ssao_VS|ssao_vs|vs"
    "SsaoRendering.slang|Ssao_PS|ssao_ps|ps"
    "LPVInjectRendering.slang|LPV_Inject_VS|lpvinject_vs|vs"
    "LPVInjectRendering.slang|LPV_Inject_PS|lpvinject_ps|ps"
    "LPVPropagationRendering.slang|LPV_Propagation_VS|lpvpropagation_vs|vs"
    "LPVPropagationRendering.slang|LPV_Propagation_PS|lpvpropagation_ps|ps"
    "WaterMarkRendering.slang|WaterMark_VS|watermark_vs|vs"
    "WaterMarkRendering.slang|WaterMark_PS|watermark_ps|ps"
    "Particles/ParticleRender.slang|ParticleRender_VS|particle_vs|vs"
    "Particles/ParticleRender.slang|ParticleRender_PS|particle_ps|ps"
)
set(MULTI_SOURCES "")
foreach(entry ${MULTI_ENTRY_SHADERS})
    string(REPLACE "|" ";" parts "${entry}")
    list(GET parts 0 src)
    list(APPEND MULTI_SOURCES "${src}")
endforeach()
list(REMOVE_DUPLICATES MULTI_SOURCES)

# 单入口着色器
file(GLOB_RECURSE SLANG_FILES "${SOURCE_DIR}/*.slang")
list(SORT SLANG_FILES)

foreach(f ${SLANG_FILES})
    file(RELATIVE_PATH rel "${SOURCE_DIR}" "${f}")
    if(rel MATCHES "^shared/" OR rel MATCHES "ParticleCommon")
        continue()
    endif()
    list(FIND MULTI_SOURCES "${rel}" is_multi)
    if(is_multi GREATER_EQUAL 0)
        continue()
    endif()
    message(STATUS "[depend] ${rel}")
    execute_process(COMMAND "${COMPILER}" --input ${rel} --depend
        WORKING_DIRECTORY "${SOURCE_DIR}" RESULT_VARIABLE r)
    if(NOT r EQUAL 0)
        message(FATAL_ERROR "depend failed: ${rel}")
    endif()
    message(STATUS "[compile] ${rel}")
    execute_process(COMMAND "${COMPILER}" --input=${rel} --target=${TARGET} --define=SEEK_HLSL=1
        WORKING_DIRECTORY "${SOURCE_DIR}" RESULT_VARIABLE r)
    if(NOT r EQUAL 0)
        message(FATAL_ERROR "compile failed: ${rel}")
    endif()
endforeach()

# 多入口点着色器 depend
set(DONE "")
foreach(entry ${MULTI_ENTRY_SHADERS})
    string(REPLACE "|" ";" parts "${entry}")
    list(GET parts 0 src)
    list(FIND DONE "${src}" done)
    if(done EQUAL -1)
        message(STATUS "[depend] ${src}")
        execute_process(COMMAND "${COMPILER}" --input ${src} --depend
            WORKING_DIRECTORY "${SOURCE_DIR}" RESULT_VARIABLE r)
        if(NOT r EQUAL 0)
            message(FATAL_ERROR "depend failed: ${src}")
        endif()
        list(APPEND DONE "${src}")
    endif()
endforeach()

# 多入口点着色器 compile
foreach(entry ${MULTI_ENTRY_SHADERS})
    string(REPLACE "|" ";" parts "${entry}")
    list(GET parts 0 src)
    list(GET parts 1 out)
    list(GET parts 2 entry_name)
    list(GET parts 3 stage)
    message(STATUS "[multi:${stage}] ${src} -> ${out}")
    execute_process(COMMAND "${COMPILER}" --input=${src} --output=${out} --stage=${stage} --entry=${entry_name} --target=${TARGET} --define=SEEK_HLSL=1
        WORKING_DIRECTORY "${SOURCE_DIR}" RESULT_VARIABLE r)
    if(NOT r EQUAL 0)
        message(FATAL_ERROR "multi-entry compile failed: ${src}/${out}")
    endif()
endforeach()

message(STATUS "All shaders: OK")
