# 顺序编译所有着色器，避免 MSBuild 并行批处理导致的 ShaderCompiler 多实例文件竞态
# 支持增量编译：对比源文件与 .compile tag 时间戳，跳过未修改的着色器
set(COMPILER "${CMAKE_ARGV3}")
set(TARGET "${CMAKE_ARGV4}")
set(SOURCE_DIR "${CMAKE_ARGV5}")
set(STAMP_FILE "${CMAKE_ARGV6}")
# 从 stamp 文件路径推导 tag 目录和生成文件目录
# stamp 路径格式: ${SEEK_GENERATED_SHADER_DIR}/.tag/.shaders_complete.tag
if(NOT STAMP_FILE OR STAMP_FILE STREQUAL "")
    # 兼容旧调用方式（未传 stamp 文件路径）：从 SOURCE_DIR 推导
    get_filename_component(PROJECT_DIR "${SOURCE_DIR}" DIRECTORY)
    set(GENERATED_DIR "${PROJECT_DIR}/build/generated/shader")
    set(TAG_BASE_DIR "${GENERATED_DIR}/.tag")
    message(STATUS "[warning] 未传入 stamp 文件路径，使用推导路径: ${GENERATED_DIR}")
else()
    get_filename_component(TAG_BASE_DIR "${STAMP_FILE}" DIRECTORY)
    get_filename_component(GENERATED_DIR "${TAG_BASE_DIR}" DIRECTORY)
endif()
set(TAG_COMPILE_DIR "${TAG_BASE_DIR}/.compile")
set(META_DIR "${GENERATED_DIR}/.meta")
set(TARGET_DIR "${GENERATED_DIR}/${TARGET}")

# 辅助函数：验证着色器输出文件是否存在（元信息头 + 目标聚合头）
function(verify_shader_output name)
    set(meta_header "${META_DIR}/${name}.h")
    set(aggregate_header "${TARGET_DIR}/${name}.h")
    if(NOT EXISTS "${meta_header}")
        message(FATAL_ERROR "输出文件缺失: ${meta_header} —— ShaderCompiler 未生成此文件，请检查编译日志")
    endif()
    if(NOT EXISTS "${aggregate_header}")
        message(FATAL_ERROR "输出文件缺失: ${aggregate_header} —— ShaderCompiler 未生成此文件，请检查编译日志")
    endif()
endfunction()

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

    # 获取文件名（不含扩展名）
    get_filename_component(basename "${rel}" NAME_WLE)

    # 增量编译检查：如果 .compile tag 文件存在且比源文件新，且输出文件存在，则跳过
    set(compile_tag "${TAG_COMPILE_DIR}/${basename}.tag")
    set(skip FALSE)
    if(EXISTS "${compile_tag}")
        if(NOT "${f}" IS_NEWER_THAN "${compile_tag}")
            set(skip TRUE)
        endif()
    endif()
    # 即使 tag 表示可以跳过，也要验证输出文件确实存在
    if(skip)
        set(meta_header "${META_DIR}/${basename}.h")
        set(aggregate_header "${TARGET_DIR}/${basename}.h")
        if(EXISTS "${meta_header}" AND EXISTS "${aggregate_header}")
            message(STATUS "[up-to-date] ${rel}")
            continue()
        else()
            message(STATUS "[stale-output] ${rel} —— tag 存在但输出文件丢失，重新编译")
        endif()
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
    # 编译完成后，验证输出文件已生成
    verify_shader_output("${basename}")
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

# 多入口点着色器 compile（带增量检查）
foreach(entry ${MULTI_ENTRY_SHADERS})
    string(REPLACE "|" ";" parts "${entry}")
    list(GET parts 0 src)
    list(GET parts 1 out)
    list(GET parts 2 entry_name)
    list(GET parts 3 stage)

    # 增量编译检查：多入口点着色器的 tag 基于输出名称
    set(compile_tag "${TAG_COMPILE_DIR}/${out}.tag")
    set(src_abs "${SOURCE_DIR}/${src}")
    set(skip FALSE)
    if(EXISTS "${compile_tag}")
        if(NOT "${src_abs}" IS_NEWER_THAN "${compile_tag}")
            set(skip TRUE)
        endif()
    endif()
    # 即使 tag 表示可以跳过，也要验证输出文件确实存在
    if(skip)
        set(meta_header "${META_DIR}/${out}.h")
        set(aggregate_header "${TARGET_DIR}/${out}.h")
        if(EXISTS "${meta_header}" AND EXISTS "${aggregate_header}")
            message(STATUS "[up-to-date] ${src} -> ${out}")
            continue()
        else()
            message(STATUS "[stale-output] ${src} -> ${out} —— tag 存在但输出文件丢失，重新编译")
        endif()
    endif()

    message(STATUS "[multi:${stage}] ${src} -> ${out}")
    execute_process(COMMAND "${COMPILER}" --input=${src} --output=${out} --stage=${stage} --entry=${entry_name} --target=${TARGET} --define=SEEK_HLSL=1
        WORKING_DIRECTORY "${SOURCE_DIR}" RESULT_VARIABLE r)
    if(NOT r EQUAL 0)
        message(FATAL_ERROR "multi-entry compile failed: ${src}/${out}")
    endif()
    # 编译完成后，验证输出文件已生成
    verify_shader_output("${out}")
endforeach()

if(NOT STAMP_FILE STREQUAL "")
    file(TOUCH "${STAMP_FILE}")
endif()
message(STATUS "All shaders: OK")
