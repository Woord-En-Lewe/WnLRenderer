cmake_minimum_required(VERSION 3.31)
project(
    wnlrenderer
    VERSION 1.0.0
    LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

CPMAddPackage(URI "gh:fmtlib/fmt#11.2.0")
CPMAddPackage(URI "gh:g-truc/glm#1.0.1")
CPMAddPackage(URI "gh:glfw/glfw#3.4")

set(DEVELOPMENT ${WNLRENDERER_DEVELOPMENT})
option(WNLRENDERER_DEVELOPMENT "Enable development compiler definition" OFF)
set(LINTERS ${WNLMEDIACLIENT_LINTERS})
option(WNLRENDERER_LINTERS "Enable linters" OFF)

if (LINTERS)
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
    else()
        message(WARNING "clang-tidy not found. Static analysis will be skipped.")
    endif()
endif()

add_subdirectory(vendor)
add_subdirectory(src)

install(TARGETS renderer glm glm-header-only
    EXPORT renderer-targets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/renderer
)

install(EXPORT renderer-targets
    FILE
        renderer-targets.cmake
    NAMESPACE
        renderer::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/renderer
)
