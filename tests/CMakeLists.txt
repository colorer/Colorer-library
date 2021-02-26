
macro(add_sample_executable name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE colorer_lib)
  set_target_properties(${name} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED YES
      CXX_EXTENSIONS NO
      )
endmacro()

add_sample_executable(texthrd
    hrd_mapper/hrdmapper.cpp
    )

add_sample_executable(perftest
    performance/test_speed.cpp
    performance/tests.cpp
    performance/tests.h
    )
