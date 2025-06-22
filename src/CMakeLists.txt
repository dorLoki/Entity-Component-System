set(fmt_FOUND FALSE)
find_package(fmt QUIET CONFIG)

if(fmt_FOUND)
    message(STATUS "fmt found")
else()
    message(STATUS "fmt not found - fallback")

    include(FetchContent)

    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
    )

    FetchContent_MakeAvailable(fmt)
endif()

add_executable(${CMAKE_PROJECT_NAME} "main.cpp")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE fmt::fmt)
