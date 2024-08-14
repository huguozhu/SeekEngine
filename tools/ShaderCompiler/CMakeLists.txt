
cmake_minimum_required(VERSION 3.15) 
project(ShaderCompiler)
cmake_policy(SET CMP0054 NEW)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED SEEK_ROOT_DIR)
    message(FATAL_ERROR "must specify SEEK_ROOT_DIR")
endif()
if(NOT DEFINED RAPIDJSON_INCLUDE_DIR)
    message(FATAL_ERROR "must specify RAPIDJSON_INCLUDE_DIR")
endif()

set(SHADER_COMPILER_SRCS
    ShaderCompiler.cpp
    shader_helper.h
)
source_group("Source" FILES ${SHADER_COMPILER_SRCS})
add_executable(ShaderCompiler ${SHADER_COMPILER_SRCS})

target_compile_definitions(ShaderCompiler PRIVATE NOMINMAX)

target_include_directories(ShaderCompiler PRIVATE
    ${SEEK_ROOT_DIR}
    ${SEEK_ROOT_DIR}/third_party/CLI11/include
    ${SEEK_ROOT_DIR}/third_party/shaderconductor/include
    ${CMAKE_CURRENT_BINARY_DIR}/../../generated
    ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(ShaderCompiler PRIVATE ${SEEK_THIRDPARTY_DIR}/shaderconductor/libs/${TARGET_PLATFORM}/ShaderConductor.lib)

if(WIN32)
    target_link_libraries(ShaderCompiler PRIVATE d3dcompiler)
endif()

set_target_properties(ShaderCompiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

if(APPLE)
    set_target_properties(ShaderCompiler PROPERTIES
        XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD)"
        XCODE_ATTRIBUTE_VALID_ARCHS "x86_64"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path ${shaderconductor_LIB_DIRS}"
    )
endif()

message(FATAL_INFO "shaderconductor_LIB_DIRS ----- shaderconductor_LIB_DIRS")
message(FATAL_INFO $<TARGET_FILE_DIR>)
message(FATAL_INFO $<TARGET_FILE_DIR:ShaderCompiler>)
set(shaderconductor_LIB_DIRS ${SEEK_THIRDPARTY_DIR}/shaderconductor/libs/${TARGET_PLATFORM})
if(WIN32)
    add_custom_command(TARGET ShaderCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${shaderconductor_LIB_DIRS}/dxcompiler.dll      $<TARGET_FILE_DIR:ShaderCompiler>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${shaderconductor_LIB_DIRS}/ShaderConductor.dll $<TARGET_FILE_DIR:ShaderCompiler>
    )
endif()
