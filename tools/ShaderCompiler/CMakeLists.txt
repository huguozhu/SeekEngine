
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

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
    ${SEEK_ROOT_DIR}/third_party/slang/include
    ${SEEK_GENERATED_FILE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(ShaderCompiler PRIVATE
    ${SEEK_THIRDPARTY_DIR}/slang/libs/windows/slang.lib
    d3dcompiler
)


add_custom_command(TARGET ShaderCompiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SEEK_THIRDPARTY_DIR}/slang/libs/windows/slang.dll $<TARGET_FILE_DIR:ShaderCompiler>
)
