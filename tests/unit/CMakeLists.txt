cmake_minimum_required(VERSION 3.15)
project(unit_tests CXX)

set(unit_tests_SRC test_exception.cpp)

add_executable(unit_tests ${unit_tests_SRC})

target_link_libraries(unit_tests
    PRIVATE GTest::GTest GTest::Main colorer_lib)

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )