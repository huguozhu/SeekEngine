############################################ SeekEngin source files ############################################
set(ENGINE_CODE
    ${SEEK_ENGINE_SOURCE_DIR}/seek_engine.h
    
   # components
    ${SEEK_ENGINE_SOURCE_DIR}/components/entity.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/entity.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/components/component.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/mesh_component.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/mesh_component.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/components/light_component.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/light_component.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/components/scene_component.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/scene_component.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/components/camera_component.h
    ${SEEK_ENGINE_SOURCE_DIR}/components/camera_component.cpp


    # effect
    ${SEEK_ENGINE_SOURCE_DIR}/effect/draw_call.h
    ${SEEK_ENGINE_SOURCE_DIR}/effect/draw_call.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/effect/command_buffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/effect/command_buffer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/effect/parameter.h
    ${SEEK_ENGINE_SOURCE_DIR}/effect/parameter.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/effect/variable.h

    # kernel
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/kernel.h
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/context.h
    ${SEEK_ENGINE_SOURCE_DIR}/kernel/context.cpp

    # math
    ${SEEK_ENGINE_SOURCE_DIR}/math/vector.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/hash.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/rect.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/math_utility.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/math_utility.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/matrix.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/matrix.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/aabbox.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/aabbox.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/color.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/color.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/frustum.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/frustum.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/plane.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/plane.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/quad_mesh_process.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/quad_mesh_process.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/quaternion.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/quaternion.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/ray.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/ray.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/math/transform.h
    ${SEEK_ENGINE_SOURCE_DIR}/math/transform.cpp
    
    # rhi
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/format.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/format.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_framebuffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_framebuffer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/material.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_mesh.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_mesh.cpp    
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_program.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_program.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_query.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_render_buffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_definition.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_render_state.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_render_state.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_render_view.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_context.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_context.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_shader.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_shader.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/shader_param_assign_helper.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/rhi_texture.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/base/viewport.h
    

    # d3d11_rhi
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_adapter.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_adapter.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_framebuffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_framebuffer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_mesh.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_mesh.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_predeclare.h

    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_program.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_program.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_query.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_query.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_buffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_buffer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_state.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_state.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_view.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_render_view.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_rhi_context.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_rhi_context.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_rhi_context_factory.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_shader.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_shader.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_shader_param_assign_helper.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_shader_param_assign_helper.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_texture.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_texture.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_translate.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_translate.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_window.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/d3d11_rhi/d3d11_window.cpp


    # vulkan rhi
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/vulkan_rhi/vulkan_rhi.h
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/vulkan_rhi/vulkan_rhi.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/rhi/vulkan_rhi/vulkan_rhi_factory.cpp

    # resource
    ${SEEK_ENGINE_SOURCE_DIR}/resource/resource_mgr.h
    ${SEEK_ENGINE_SOURCE_DIR}/resource/resource_mgr.cpp

    # utils
    ${SEEK_ENGINE_SOURCE_DIR}/utils/file_uid.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/util.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/util.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/buffer.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/buffer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/compression.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/compression.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/dll_loader.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/dll_loader.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/error.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/error.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/exposure.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/exposure.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/image_decode.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/image_decode.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/log.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/log.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/safe_c.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/safe_c.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/timer.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/timer.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/zbase64.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/zbase64.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/utils/file.h
    ${SEEK_ENGINE_SOURCE_DIR}/utils/seek_defs.h


    # scene_manager
    ${SEEK_ENGINE_SOURCE_DIR}/scene_manager/scene_manager.h
    ${SEEK_ENGINE_SOURCE_DIR}/scene_manager/scene_manager.cpp

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

############################################ seek_engine target ############################################
set(SEEK_SHAREDLIB_NAME ${PROJECT_NAME} CACHE STRING "" FORCE)
set(SEEK_STATICLIB_NAME ${PROJECT_NAME}-static CACHE STRING "" FORCE)

if(TARGET_PLATFORM_WINDOWS)
    set(SEEK_BUILD_SHAREDLIB TRUE)
    set(SEEK_STATICLIB_DEPENDENT_LIBS dxgi d3d11 d3dcompiler dxguid Windowscodecs Ws2_32 Winmm)
endif()    


############################################ seek_engine static lib ############################################
add_library(${SEEK_STATICLIB_NAME} STATIC ${ENGINE_CODE})
add_dependencies(${SEEK_STATICLIB_NAME} GenerateShaders)
target_link_libraries(${SEEK_STATICLIB_NAME} PUBLIC
    turbojpeg
    libpng
    rapidjson
    eigen
    zlib
    ${SEEK_STATICLIB_DEPENDENT_LIBS}
)
target_include_directories(${SEEK_STATICLIB_NAME} PUBLIC ${SEEK_ENGINE_SOURCE_DIR})



############################################ seek_engine shared lib ############################################
if(SEEK_BUILD_SHAREDLIB)
    add_library(${SEEK_SHAREDLIB_NAME} SHARED ${SEEK_ENGINE_SOURCE_DIR}/fake.cpp)
    target_link_libraries(${SEEK_SHAREDLIB_NAME} PUBLIC ${SEEK_STATICLIB_NAME})
endif()