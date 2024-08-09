############################################ SeekEngin source files ############################################
set(ENGINE_CODE
    ${SEEK_ENGINE_SOURCE_DIR}/seek_engine.h
    
    # components
    #${SEEK_ENGINE_SOURCE_DIR}/components/entity.h
    #${SEEK_ENGINE_SOURCE_DIR}/components/entity.cpp

    # effect
    ${SEEK_ENGINE_SOURCE_DIR}/effect/draw_call.h
    ${SEEK_ENGINE_SOURCE_DIR}/effect/draw_call.cpp

    # kernel
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/kernel.h
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/context.h
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/context.cpp

    # math
    
    # rhi
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/rhi.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/rhi_command.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/rhi_command.cpp

    # vulkan rhi
    ${SEEK_ENGINE_SOURCE_DIR}/vulkan_rhi/vulkan_rhi.h
    ${SEEK_ENGINE_SOURCE_DIR}/vulkan_rhi/vulkan_rhi.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/vulkan_rhi/vulkan_rhi_factory.cpp

    # resource

    # utils
    ${SEEK_ENGINE_SOURCE_DIR}/utils/error.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/log.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/log.cpp

    # scene_manager

    # thread    
    ${SEEK_ENGINE_SOURCE_DIR}/thread/mutex.h
    ${SEEK_ENGINE_SOURCE_DIR}/thread/mutex.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/thread/semaphore.h
    ${SEEK_ENGINE_SOURCE_DIR}/thread/semaphore.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/thread/thread.h
    ${SEEK_ENGINE_SOURCE_DIR}/thread/thread.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/thread/thread_manager.h
    ${SEEK_ENGINE_SOURCE_DIR}/thread/thread_manager.cpp

    # wrapper
)
source_group(TREE ${SEEK_ENGINE_SOURCE_DIR}/ FILES ${ENGINE_CODE})

############################################ seek target ############################################
set(SEEK_SHAREDLIB_NAME ${PROJECT_NAME} CACHE STRING "" FORCE)
set(SEEK_STATICLIB_NAME ${PROJECT_NAME}-static CACHE STRING "" FORCE)

if(TARGET_PLATFORM_WINDOWS)
    set(SEEK_BUILD_SHAREDLIB TRUE)
    set(SEEK_STATICLIB_DEPENDENT_LIBS dxgi d3d12 d3dcompiler dxguid Windowscodecs Ws2_32 Winmm)    
endif()    


############################################ seek static lib ############################################
add_library(${SEEK_STATICLIB_NAME} STATIC ${ENGINE_CODE})
target_include_directories(${SEEK_STATICLIB_NAME} PUBLIC ${SEEK_ENGINE_SOURCE_DIR})