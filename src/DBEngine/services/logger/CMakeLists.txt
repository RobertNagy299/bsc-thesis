add_library(LoggerService SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/error_logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/status_logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/warning-logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/message-templates/context-injector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/message-templates/resolvers/error-template-resolver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/message-templates/resolvers/warning-template-resolver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/message-templates/resolvers/fatal-error-template-resolver.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/std_logger/message-templates/public-api.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)