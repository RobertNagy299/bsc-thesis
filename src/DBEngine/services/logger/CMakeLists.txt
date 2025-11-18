add_library(ErrorLogger SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/error_logger/error_logger.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)