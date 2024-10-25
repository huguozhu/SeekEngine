##################################  sample common  ##########################################
set(SAMPLE_COMMON_PROJECT_NAME SampleCommon)

set(SAMPLE_COMMON_SRCS
    ${SEEK_SAMPLES_DIR}/common/app_framework.h
    ${SEEK_SAMPLES_DIR}/common/app_framework.cpp
)

set(SAMPLE_DEPENDENT_LIBS)

add_library(${SAMPLE_COMMON_PROJECT_NAME} STATIC ${SAMPLE_COMMON_SRCS})

add_dependencies(${SAMPLE_COMMON_PROJECT_NAME} ${SEEK_SHAREDLIB_NAME})
set_target_properties(${SAMPLE_COMMON_PROJECT_NAME} PROPERTIES FOLDER "samples")

# link
target_link_libraries(${SAMPLE_COMMON_PROJECT_NAME} PUBLIC ${SAMPLE_DEPENDENT_LIBS} ${SEEK_STATICLIB_NAME})
#target_link_libraries(${SAMPLE_COMMON_PROJECT_NAME} PRIVATE IMGUI)

# include
target_include_directories(${SAMPLE_COMMON_PROJECT_NAME} PUBLIC
    ${SEEK_THIRDPARTY_DIR}/imgui/include
    ${SEEK_ENGINE_SOURCE_DIR}
)


##################################  sample  ##########################################
function(add_sample_project sample_name)
    # source code
    set(SAMPLE_CODE
        ${SEEK_SAMPLES_DIR}/${sample_name}/${sample_name}.cpp
        ${SEEK_SAMPLES_DIR}/${sample_name}/${sample_name}.h
    )

    # resource file/dir
    set(SAMPLE_RESOURCE_FILES "")
    set(EMBED_FRAMEWORKS "")

    source_group(Resources FILES ${SAMPLE_RESOURCE_FILES})
    source_group(TREE ${SEEK_SAMPLES_DIR}/${sample_name}/ FILES ${SAMPLE_CODE})
    # execute
    add_executable(${sample_name} ${SAMPLE_CODE} ${SAMPLE_RESOURCE_FILES})
    target_compile_definitions(${sample_name} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(${sample_name} PUBLIC ${SAMPLE_COMMON_PROJECT_NAME})
    # property
    set_target_properties(${sample_name} PROPERTIES FOLDER "samples")

    if (WIN32)
        set_target_properties(${sample_name}    PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    target_include_directories(${sample_name} PUBLIC 
        ${SEEK_ENGINE_SOURCE_DIR}
        ${SEEK_SAMPLES_DIR}/common
    )

endfunction(add_sample_project)

add_sample_project(01.Tutorial)
add_sample_project(02.Particles)

