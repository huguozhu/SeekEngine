

################################################ IMGUI ################################################
set(IMGUI_FILES
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imconfig.h
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui_demo.cpp
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui_draw.cpp
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui_internal.h
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui_tables.cpp
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui_widgets.cpp
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui.cpp
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imgui.h
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imstb_rectpack.h
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imstb_textedit.h
    ${SEEK_THIRDPARTY_DIR}/imgui/include/imstb_truetype.h
)
if(TARGET_PLATFORM_WINDOWS)
    list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_win32.cpp)
    list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_dx11.cpp)
elseif(TARGET_PLATFORM_MACOS)
    list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_metal.mm
                            ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_osx.mm)
elseif(TARGET_PLATFORM_IOS)
    list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_metal.mm)
endif()

add_library(IMGUI INTERFACE)
target_sources(IMGUI INTERFACE ${IMGUI_FILES})
target_include_directories(IMGUI INTERFACE
    ${SEEK_THIRDPARTY_DIR}/imgui/include
    ${SEEK_THIRDPARTY_DIR}/imgui/include/backends
    ${SEEK_THIRDPARTY_DIR}/glfw/include
)

################################################ GLFW ################################################
if(SEEK_WINDOWS_OPENGLES)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER "third_party/glfw")
    set_target_properties(update_mappings PROPERTIES FOLDER "third_party/glfw")
    add_library(glfw-static INTERFACE)
    target_link_libraries(glfw-static INTERFACE glfw)
    target_include_directories(glfw-static INTERFACE ${SEEK_THIRDPARTY_DIR}/glfw/include)
endif()