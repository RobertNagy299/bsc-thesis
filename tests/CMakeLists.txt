find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test_parser test_parser.cpp)

target_link_libraries(test_parser PRIVATE
    SQLCore
    GTest::gtest_main   # <-- provides main()
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_parser)
