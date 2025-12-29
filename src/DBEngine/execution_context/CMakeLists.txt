add_library(ExecutionContext SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/src/constructor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/destructor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/setters.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/getters.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/erasers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/initializers.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)