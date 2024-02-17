add_executable(afv)

target_sources(afv
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/afv.m.cpp
)

target_link_libraries(afv
    PRIVATE
        afvbuf
        fmt::fmt
        project-options
)

target_compile_definitions(afv
    PRIVATE
        _CRT_SECURE_NO_WARNINGS)