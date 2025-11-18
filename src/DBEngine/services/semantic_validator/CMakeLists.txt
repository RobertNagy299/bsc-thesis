

add_library(SemanticValidator SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/insert_validator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/select_validator.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)

target_link_libraries(SemanticValidator Utils)
