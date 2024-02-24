add_executable(afv)

set(AFV_PLATFORM_SOURCES "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND AFV_PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/afv_windows.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND AFV_PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/afv_linux.cpp)
endif()

target_sources(afv
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/afv.m.cpp
        ${AFV_PLATFORM_SOURCES}
)

set(AFV_PLATFORM_LIBRARIES "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND AFV_PLATFORM_LIBRARIES Curses::Curses)
endif()

target_link_libraries(afv
    PRIVATE
        afvbuf
        ${AFV_PLATFORM_LIBRARIES}
        fmt::fmt
        project-options
)

set(AFV_PLATFORM_DEFINITIONS "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND AFV_PLATFORM_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
endif()

target_compile_definitions(afv
    PRIVATE
        ${AFV_PLATFORM_DEFINITIONS})