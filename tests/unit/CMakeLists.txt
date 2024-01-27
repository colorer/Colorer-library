cmake_minimum_required(VERSION 3.15)
project(unit_tests CXX)

set(unit_tests_SRC
    test_main.cpp
    test_exception.cpp
    test_filetype.cpp
    test_environment.cpp
    test_xmlinputsource.cpp
    test_hrcparsing.cpp)

add_executable(unit_tests ${unit_tests_SRC})

target_link_libraries(unit_tests PRIVATE colorer_lib)
target_include_directories(unit_tests PUBLIC ${CMAKE_SOURCE_DIR}/external)

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )