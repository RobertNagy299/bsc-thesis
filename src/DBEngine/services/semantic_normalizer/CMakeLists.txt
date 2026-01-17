add_library(SemanticNormalizer SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/src/insert.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/select.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/where.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public-api.hpp
)

target_link_libraries(SemanticNormalizer Utils)