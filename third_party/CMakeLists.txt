

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
list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_win32.cpp)
list(APPEND IMGUI_FILES ${SEEK_THIRDPARTY_DIR}/imgui/include/backends/imgui_impl_dx11.cpp)

add_library(IMGUI STATIC ${IMGUI_FILES})
target_include_directories(IMGUI PUBLIC
    ${SEEK_THIRDPARTY_DIR}/imgui/include
    ${SEEK_THIRDPARTY_DIR}/imgui/include/backends
    ${SEEK_THIRDPARTY_DIR}/glfw/include
)

set_target_properties(IMGUI PROPERTIES
    FOLDER "third_party/imgui"
)
################################################ libpng ################################################
add_library(libpng INTERFACE)
target_include_directories(libpng INTERFACE ${SEEK_THIRDPARTY_DIR}/libpng/include)
target_link_libraries(libpng INTERFACE ${SEEK_THIRDPARTY_DIR}/libpng/libs/windows/libpng16_static.lib)

################################################ turbojpeg ################################################
add_library(turbojpeg INTERFACE)
target_include_directories(turbojpeg INTERFACE ${SEEK_THIRDPARTY_DIR}/libjpeg-turbo/include)
target_link_libraries(turbojpeg INTERFACE ${SEEK_THIRDPARTY_DIR}/libjpeg-turbo/libs/windows/turbojpeg-static.lib)

################################################ rapidjson ################################################
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${SEEK_THIRDPARTY_DIR}/rapidjson/include)

################################################ eigen ################################################
add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE ${SEEK_THIRDPARTY_DIR}/eigen/include)

################################################ zlib ################################################
add_library(zlib INTERFACE)
target_include_directories(zlib INTERFACE ${SEEK_THIRDPARTY_DIR}/zlib/include)
target_link_libraries(zlib INTERFACE ${SEEK_THIRDPARTY_DIR}/zlib/libs/windows/Zlib_19.lib)