if (DEVELOPMENT)
    target_compile_definitions(renderer INTERFACE DEVELOPMENT)
    add_executable(app main.cpp)
    target_sources(app
        PRIVATE
            vert.h
            frag.h
    )
    target_link_libraries(app PRIVATE renderer)
    target_compile_features(app PRIVATE cxx_std_20)
endif()

