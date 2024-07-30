set(TEST_SOURCES
    common/number_converter_test.cpp
    # common/memory_context_test.cpp
    nodes/json_value_test.cpp
    # parser/json_parser_test.cpp
    parser/number_parser_test.cpp
)

foreach(test_source ${TEST_SOURCES})
    get_filename_component(target ${test_source} NAME_WE)
    add_executable(${target} ${test_source})
    target_include_directories(${target} PRIVATE ${GTEST_INCLUDE_DIR})
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
    target_link_libraries(${target} PRIVATE ${GTEST_LIBRARIES})

    add_test(NAME ${target} COMMAND ${target})
endforeach()