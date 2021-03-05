# IDE support

#====================================================
# Set configuration types
#====================================================
if(NOT MSVC_IDE)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
else()
  #target_compile_options cannot set parameters for all configurations in MSVC
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
endif()
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")