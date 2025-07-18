if (DEVELOPMENT)
    target_compile_definitions(renderer INTERFACE DEVELOPMENT)
    add_executable(app)
    target_sources(app
        PRIVATE
            main.cpp

        PRIVATE FILE_SET HEADERS FILES
            vert.h
            frag.h
            window.h
    )
    target_link_libraries(app
        PRIVATE
            glfw
            renderer
    )
    target_compile_features(app PRIVATE cxx_std_20)
endif()

