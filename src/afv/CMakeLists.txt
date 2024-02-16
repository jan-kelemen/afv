add_executable(afv)

target_sources(afv
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/afv.m.cpp
)

target_link_libraries(afv
    PRIVATE
        fmt::fmt
        project-options
)

if (AFV_BUILD_TESTS)
    add_executable(afv_test)

    target_sources(afv_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/afv.t.cpp
    )

    target_link_libraries(afv_test
        PRIVATE
            Catch2::Catch2WithMain
            project-options
    )

    include(Catch)
    catch_discover_tests(afv_test)
endif()
