add_library(afvbuf)

target_sources(afvbuf
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/afvbuf_text_buffer.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/afvbuf_text_buffer.cpp
)

target_include_directories(afvbuf
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(afvbuf
    PRIVATE
        project-options
)

if (AFV_BUILD_TESTS)
    add_executable(afvbuf_test)

    target_sources(afvbuf_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/afvbuf_text_buffer.t.cpp
    )

    target_link_libraries(afvbuf_test
        PRIVATE
            afvbuf
            Catch2::Catch2WithMain
            project-options
    )

    include(Catch)
    catch_discover_tests(afvbuf_test)
endif()
