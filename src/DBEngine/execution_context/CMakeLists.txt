add_library(ExecutionContext SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/src/init.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/destructor.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)