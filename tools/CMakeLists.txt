# ShaderCompiler
set(RAPIDJSON_INCLUDE_DIR "${SEEK_ROOT_DIR}/third_party/rapidjson/include")

if(TARGET_PLATFORM_WINDOWS OR TARGET_PLATFORM_MACOS OR TARGET_PLATFORM_LINUX)
    add_subdirectory(ShaderCompiler)
endif()

if(WIN32)
    set(ShaderCompiler_bin ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler/ShaderCompiler.exe CACHE STRING "" FORCE)
else()
    set(ShaderCompiler_bin ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler/ShaderCompiler CACHE STRING "" FORCE)
endif()

set_target_properties(ShaderCompiler PROPERTIES FOLDER "tools")