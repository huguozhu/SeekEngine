# ShaderCompiler
set(RAPIDJSON_INCLUDE_DIR "${SEEK_ROOT_DIR}/third_party/rapidjson/include")

add_subdirectory(ShaderCompiler)

set(ShaderCompiler_bin ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler/ShaderCompiler.exe CACHE STRING "" FORCE)

set_target_properties(ShaderCompiler PROPERTIES FOLDER "tools")