add_library(Utils SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/types/types.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/utils/string_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/column_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/insert_utils.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/utils_public_api.hpp
)
