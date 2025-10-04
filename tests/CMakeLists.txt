find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test_parser test_parser.cpp)

target_link_libraries(test_parser PRIVATE
    SQLCore
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_parser)

# ========================================
# Valgrind Static Target
# ========================================
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    message(STATUS "Valgrind found: ${VALGRIND_EXECUTABLE}")

    set(TEST_LIST
        ParserTest.CreateUntypedTableCorrectly
        ParserTest.CreateUntypedTableCorrectlyWithColModifiers
        ParserTest.CreateTableWithoutTableKeyword
    )

    add_custom_target(run_valgrind_tests
        COMMENT "Running selected GoogleTest cases under Valgrind"
        DEPENDS test_parser
        COMMAND ${CMAKE_COMMAND}
            -DVALGRIND_EXECUTABLE=${VALGRIND_EXECUTABLE}
            -DOUTPUT_DIR=${CMAKE_BINARY_DIR}/tests/valgrind_logs
            -DTEST_BINARY=$<TARGET_FILE:test_parser>
            -DTEST_LIST="${TEST_LIST}"
            -P ${CMAKE_SOURCE_DIR}/cmake/run_valgrind_static.cmake
    )
else()
    message(WARNING "Valgrind not found — skipping Valgrind targets")
endif()
