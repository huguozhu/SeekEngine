cmake_minimum_required(VERSION 3.30)

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0091 OLD)

project(SeekEngine)

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



add_subdirectory(third_party)
add_subdirectory(engine)
add_subdirectory(samples)