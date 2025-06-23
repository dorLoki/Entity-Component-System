# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(GTest_FOUND FALSE)
find_package(GTest QUIET CONFIG)

if(GTest_FOUND)
    message(STATUS "GTest found")
else()
    message(STATUS "GTest not found - fallback")

    include(FetchContent)

    FetchContent_Declare(
        GTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(GTest)
endif()

add_executable(
  Testing
  v3/ecs_test.cpp
  v4/test.cpp
  v5/test.cpp
)

target_link_libraries(Testing PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)


include(GoogleTest)
gtest_discover_tests(Testing)