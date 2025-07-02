cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 20)
cmake_policy(SET CMP0054 NEW)
#set_property(TARGET tgt PROPERTY CXX_STANDARD 20)
 
project(seek_engine VERSION 0.0.1 DESCRIPTION "Seek Rendering Engine" HOMEPAGE_URL "https:://github.com/huguozhu/SeekEngine")

set(SHADER_DEPEND_FILE_SUFFIX ".depend")
set(SHADER_REFLECT_FILE_SUFFIX ".reflect")
set(SHADER_META_FILE_SUFFIX ".meta")
set(SHADER_TAG_FILE_SUFFIX ".tag")
set(SHADER_COMPILE_FILE_SUFFIX ".compile")
set(SHADER_DEBUG_DIR_SUFFIX "_debug")

########################### target platform & arch ######################
if(WIN32)
    set(TARGET_PLATFORM_WINDOWS 1)
    set(TARGET_PLATFORM "windows")
elseif(APPLE)
    set(TARGET_PLATFORM_APPLE 1)
    if(NOT IOS)
        set(TARGET_PLATFORM_MACOS 1)
        set(TARGET_PLATFORM "macos")
    else()
        set(TARGET_PLATFORM_IOS 1)
        set(TARGET_PLATFORM "ios")
    endif()
elseif(ANDROID)
    set(TARGET_PLATFORM_ANDROID 1)
    set(TARGET_PLATFORM "android")
elseif(EMSCRIPTEN)
    set(TARGET_PLATFORM_WEB 1)
    set(TARGET_PLATFORM "web")
else()
    set(TARGET_PLATFORM_LINUX 1)
    set(TARGET_PLATFORM "linux")
endif()


if(TARGET_PLATFORM_WINDOWS)
    add_definitions(-DSEEK_PLATFORM_WINDOWS)
    add_definitions(-DNOMINMAX) # disable macro min/max in Windows.h
elseif(TARGET_PLATFORM_MACOS)
    add_definitions(-DSEEK_PLATFORM_MAC -DSEEK_PLATFORM_MACOS)
elseif(TARGET_PLATFORM_IOS)
    add_definitions(-DSEEK_PLATFORM_IOS)
elseif(TARGET_PLATFORM_LINUX)
    add_definitions(-DSEEK_PLATFORM_LINUX)
elseif(TARGET_PLATFORM_ANDROID)
    add_definitions(-DSEEK_PLATFORM_ANDROID)
elseif(TARGET_PLATFORM_WEB)
    add_definitions(-DSEEK_PLATFORM_WEB)
endif()


# code path
set(SEEK_ROOT_DIR                "${CMAKE_CURRENT_LIST_DIR}")
set(SEEK_SHADER_SOURCE_DIR       "${SEEK_ROOT_DIR}/shader")
set(SEEK_ENGINE_SOURCE_DIR       "${SEEK_ROOT_DIR}/engine")
set(SEEK_SAMPLES_DIR             "${SEEK_ROOT_DIR}/samples")
set(SEEK_TOOLS_DIR               "${SEEK_ROOT_DIR}/tools")
set(SEEK_THIRDPARTY_DIR          "${SEEK_ROOT_DIR}/third_party")
set(SEEK_DEPENDENCIES_DIR        "${SEEK_ROOT_DIR}/dependencies")

set(SEEK_GENERATED_FILE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(SEEK_GENERATED_SHADER_DIR    "${SEEK_GENERATED_FILE_DIR}/shader")
set(SEEK_GENERATED_DEPEND_DIR    "${SEEK_GENERATED_SHADER_DIR}/${SHADER_DEPEND_FILE_SUFFIX}")
set(SEEK_GENERATED_META_DIR      "${SEEK_GENERATED_SHADER_DIR}/${SHADER_META_FILE_SUFFIX}")
set(SEEK_GENERATED_TAG_DIR       "${SEEK_GENERATED_SHADER_DIR}/${SHADER_TAG_FILE_SUFFIX}")

configure_file(${SEEK_ROOT_DIR}/seek.config.h.in ${SEEK_GENERATED_FILE_DIR}/seek.config.h @ONLY)

######################### global include path #########################
include_directories(${SEEK_ROOT_DIR})
include_directories(${SEEK_ROOT_DIR}/common/include)
include_directories(${SEEK_GENERATED_FILE_DIR})


add_definitions(-DSEEK_CPP)


######################### function definitions #########################
function(create_directory folder_path)
    if(NOT EXISTS ${folder_path})
        file(MAKE_DIRECTORY ${folder_path})
        message(STATUS "MAKE_DIRECTORY ${folder_path}")
    endif()
endfunction()



######################### add_subdirectory #########################
add_subdirectory(third_party)
add_subdirectory(tools)
add_subdirectory(shader)
add_subdirectory(engine)
add_subdirectory(samples)