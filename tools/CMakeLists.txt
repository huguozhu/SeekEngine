# ShaderCompiler
set(RAPIDJSON_INCLUDE_DIR "${DVF_ROOT_DIR}/../../dependencies/rapidjson/include")

if(TARGET_PLATFORM_WINDOWS OR TARGET_PLATFORM_MACOS OR TARGET_PLATFORM_LINUX)

    add_subdirectory(ShaderCompiler)

else()

    message("execute_process ShaderCompiler")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(SHADER_CMAKE_PARAM -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++)
        set(SHADER_CMAKE_BUILD_PARAM)#-- -quiet)
    else()
        # prebuild dxc library is compiled by gcc, but the default compiler on Android build server is clang.
        # this may cause some strange problems, so we force to use gcc compiler
        set(SHADER_CMAKE_PARAM -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++)
        set(SHADER_CMAKE_BUILD_PARAM)
    endif()
    message(STATUS "SHADER_CMAKE_PARAM ${SHADER_CMAKE_PARAM}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/ShaderCompiler
                                -B ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler
                                -DDVF_ROOT_DIR=${DVF_ROOT_DIR}
                                -DRAPIDJSON_INCLUDE_DIR=${RAPIDJSON_INCLUDE_DIR}
                                ${SHADER_CMAKE_PARAM}
        COMMAND_ERROR_IS_FATAL ANY
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler --config Release --target ShaderCompiler ${SHADER_CMAKE_BUILD_PARAM}
        COMMAND_ERROR_IS_FATAL ANY
    )

    add_custom_target(ShaderCompiler)

endif()

if(WIN32)
    set(ShaderCompiler_bin ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler/ShaderCompiler.exe CACHE STRING "" FORCE)
else()
    set(ShaderCompiler_bin ${CMAKE_CURRENT_BINARY_DIR}/ShaderCompiler/ShaderCompiler CACHE STRING "" FORCE)
endif()

set_target_properties(ShaderCompiler PROPERTIES FOLDER "tools")