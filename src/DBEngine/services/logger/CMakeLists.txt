add_library(LoggerService SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/error_logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/status_logger.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)