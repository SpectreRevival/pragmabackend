enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE GTest::gtest)

file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
list(REMOVE_ITEM TEST_FILES "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp")

target_sources(tests PRIVATE ${TEST_FILES})
add_dependencies(tests pragmabackend)

include(GoogleTest)
gtest_discover_tests(tests)