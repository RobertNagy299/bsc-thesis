

add_library(SemanticValidator SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/where.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/insert.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/select.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validators/delete.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)

target_link_libraries(SemanticValidator Utils)
