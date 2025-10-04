if(NOT DEFINED VALGRIND_EXECUTABLE OR NOT DEFINED TEST_BINARY)
    message(FATAL_ERROR "VALGRIND_EXECUTABLE and TEST_BINARY must be defined")
endif()

file(MAKE_DIRECTORY ${OUTPUT_DIR})

set(TEST_LIST
    ParserTest.CreateUntypedTableCorrectly
    ParserTest.CreateUntypedTableCorrectlyWithColModifiers
    ParserTest.CreateTableWithoutTableKeyword
)

# Iterate over each test individually
foreach(test_name IN LISTS TEST_LIST)
    # sanitize filename for log
    string(REPLACE "." "_" safe_name ${test_name})
    string(REPLACE "/" "_" safe_name ${safe_name})

    message(STATUS "→ Valgrind running on ${test_name}")

    execute_process(
        COMMAND ${VALGRIND_EXECUTABLE}
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --error-exitcode=1
            --log-file=${OUTPUT_DIR}/${safe_name}.log
            ${TEST_BINARY} --gtest_filter=${test_name}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
    )

    if(NOT result EQUAL 0)
        message(WARNING "Valgrind reported leaks in ${test_name} (exit code ${result})")
    endif()
endforeach()

message(STATUS "Valgrind reports saved under: ${OUTPUT_DIR}")
