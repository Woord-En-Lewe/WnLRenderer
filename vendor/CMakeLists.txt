add_subdirectory(glad)

install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
