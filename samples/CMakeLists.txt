


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
    target_link_libraries(${sample_name} PUBLIC ${SEEK_STATICLIB_NAME})
    # property
    set_target_properties(${sample_name} PROPERTIES FOLDER "samples")

    if (WIN32)
        set_target_properties(${sample_name}    PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    target_include_directories(${sample_name} PUBLIC ${SEEK_ENGINE_SOURCE_DIR})

endfunction(add_sample_project)

add_sample_project(01.Tutorial)

