add_library(renderer INTERFACE)
target_sources(renderer
    PUBLIC FILE_SET HEADERS FILES
        renderer.h
        window.h
        vert.h
        frag.h
)

target_include_directories(renderer
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(renderer
    INTERFACE
        fmt::fmt
        glfw
        glm::glm

        glad
)

set_target_properties(renderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

if (DEVELOPMENT)
    target_compile_definitions(renderer INTERFACE DEVELOPMENT)
    add_executable(app main.cpp)
    target_link_libraries(app PRIVATE renderer)
    target_compile_features(app PRIVATE cxx_std_20)
endif()

