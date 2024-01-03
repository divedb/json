function (add_one_test target)
    add_executable(${target} "${target}.cc")
    target_link_libraries(${target} PRIVATE ${GTEST_LIBRARIES})
endfunction()

function (add_tests)
    foreach(test_name ${ARGN})
    add_one_test(${test_name})
    endforeach()
endfunction()

# add_tests(utf_test util_test string_parser_test)
add_tests(pipe_test value_test number_parser_test)
