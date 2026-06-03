cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0054 NEW)
 
project(seek_engine VERSION 0.0.1 DESCRIPTION "Seek Rendering Engine" HOMEPAGE_URL "https://github.com/huguozhu/SeekEngine")

set(SHADER_DEPEND_FILE_SUFFIX ".depend")
set(SHADER_REFLECT_FILE_SUFFIX ".reflect")
set(SHADER_META_FILE_SUFFIX ".meta")
set(SHADER_TAG_FILE_SUFFIX ".tag")
set(SHADER_COMPILE_FILE_SUFFIX ".compile")
set(SHADER_DEBUG_DIR_SUFFIX "_debug")

add_compile_definitions(SEEK_PLATFORM_WINDOWS NOMINMAX SEEK_CPP)


# code path
set(SEEK_ROOT_DIR                "${CMAKE_CURRENT_LIST_DIR}")
set(SEEK_SHADER_SOURCE_DIR       "${SEEK_ROOT_DIR}/shader")
set(SEEK_ENGINE_SOURCE_DIR       "${SEEK_ROOT_DIR}/engine")
set(SEEK_SAMPLES_DIR             "${SEEK_ROOT_DIR}/samples")
set(SEEK_TOOLS_DIR               "${SEEK_ROOT_DIR}/tools")
set(SEEK_THIRDPARTY_DIR          "${SEEK_ROOT_DIR}/third_party")

set(SEEK_GENERATED_FILE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(SEEK_GENERATED_SHADER_DIR    "${SEEK_GENERATED_FILE_DIR}/shader")
set(SEEK_GENERATED_DEPEND_DIR    "${SEEK_GENERATED_SHADER_DIR}/${SHADER_DEPEND_FILE_SUFFIX}")
set(SEEK_GENERATED_META_DIR      "${SEEK_GENERATED_SHADER_DIR}/${SHADER_META_FILE_SUFFIX}")
set(SEEK_GENERATED_TAG_DIR       "${SEEK_GENERATED_SHADER_DIR}/${SHADER_TAG_FILE_SUFFIX}")

configure_file(${SEEK_ROOT_DIR}/seek.config.h.in ${SEEK_GENERATED_FILE_DIR}/seek.config.h @ONLY)

######################### 自动查找 Vulkan SDK 路径 #########################
# 优先使用环境变量 VULKAN_SDK，否则在常见安装目录下搜索最新版本
if(DEFINED ENV{VULKAN_SDK} AND EXISTS "$ENV{VULKAN_SDK}")
    set(VULKAN_SDK_DIR "$ENV{VULKAN_SDK}")
else()
    # 在 C:/VulkanSDK 目录下查找最新版本
    set(_vulkan_sdk_base "C:/VulkanSDK")
    if(EXISTS "${_vulkan_sdk_base}")
        file(GLOB _vulkan_versions LIST_DIRECTORIES true "${_vulkan_sdk_base}/*")
        set(_latest_version "")
        foreach(_dir ${_vulkan_versions})
            if(IS_DIRECTORY "${_dir}")
                get_filename_component(_ver "${_dir}" NAME)
                # 版本号格式: x.y.z.w 或 x.y.z
                if(_ver MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+")
                    if(NOT _latest_version OR _ver VERSION_GREATER _latest_version)
                        set(_latest_version "${_ver}")
                    endif()
                endif()
            endif()
        endforeach()
        if(_latest_version)
            set(VULKAN_SDK_DIR "${_vulkan_sdk_base}/${_latest_version}")
        endif()
    endif()
endif()

# 验证找到的路径
if(NOT VULKAN_SDK_DIR OR NOT EXISTS "${VULKAN_SDK_DIR}")
    message(FATAL_ERROR "找不到 Vulkan SDK。请安装 Vulkan SDK 或设置 VULKAN_SDK 环境变量。")
else()
    message(STATUS "使用 Vulkan SDK: ${VULKAN_SDK_DIR}")
endif()

######################### global include path #########################
include_directories(${SEEK_ROOT_DIR})
include_directories(${SEEK_GENERATED_FILE_DIR})



######################### add_subdirectory #########################
add_subdirectory(third_party)
add_subdirectory(tools)
add_subdirectory(shader)
add_subdirectory(engine)
add_subdirectory(samples)