cmake_minimum_required(VERSION 3.20)

project("sample-application" VERSION 0.0.1 DESCRIPTION "A sample application made using Zero/Engine")

add_subdirectory(ZEngine)

set(APP_EXE game)

add_executable(${APP_EXE})
target_sources(${APP_EXE}
    PRIVATE
        main.cpp
)
target_link_libraries(${APP_EXE} PRIVATE ZEngine)
