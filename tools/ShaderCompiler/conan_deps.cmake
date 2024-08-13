include_guard(GLOBAL)

set(conan_cfg GENERATORS cmake_find_package)
if(WIN32)
    list(APPEND conan_cfg REQUIRES shaderconductor/0.3.b46_windows@zoom/testing)
elseif(APPLE)
    list(APPEND conan_cfg REQUIRES shaderconductor/0.3.b29_mac@zoom/testing)
else()
    execute_process(COMMAND bash -c "source /etc/os-release && echo $NAME"       TIMEOUT 5 OUTPUT_VARIABLE LINUX_OS_NAME     OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND bash -c "source /etc/os-release && echo $VERSION_ID" TIMEOUT 5 OUTPUT_VARIABLE LINUX_OS_VERSION  OUTPUT_STRIP_TRAILING_WHITESPACE)

    if("${LINUX_OS_NAME}${LINUX_OS_VERSION}" STREQUAL "Ubuntu16.04")
        set(LINUX_OS_DISTRO "u16")
    elseif("${LINUX_OS_NAME}${LINUX_OS_VERSION}" STREQUAL "Ubuntu18.04")
        set(LINUX_OS_DISTRO "u18")
    elseif("${LINUX_OS_NAME}${LINUX_OS_VERSION}" STREQUAL "CentOS Linux7")
        set(LINUX_OS_DISTRO "ct7")
    else()
        set(LINUX_OS_DISTRO "u16") # android is compiled in docker-ct7
    endif()
    message(STATUS "LINUX_OS_NAME:${LINUX_OS_NAME},LINUX_OS_VERSION:${LINUX_OS_VERSION},LINUX_OS_DISTRO:${LINUX_OS_DISTRO}")

    list(APPEND conan_cfg REQUIRES shaderconductor/0.3.b42_linux@zoom/testing OPTIONS shaderconductor:distro=${LINUX_OS_DISTRO})
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/conan.cmake)
conan_cmake_configure(${conan_cfg})
conan_cmake_autodetect(settings BUILD_TYPE Release)
conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_BINARY_DIR}
    SETTINGS "arch=x86_64"
    SETTINGS "build_type=Release"
    ${settings_platform}
)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_MODULE_PATH})

find_package(shaderconductor REQUIRED)
