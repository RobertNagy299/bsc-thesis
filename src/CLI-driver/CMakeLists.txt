add_library(CLIDriver SHARED

  ${CMAKE_CURRENT_SOURCE_DIR}/src/runcli.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public-api.hpp
)

target_link_libraries(CLIDriver
  PRIVATE
    SemanticValidator 
    SemanticNormalizer
    LoggerService
    FileHandler
    ExecutionContext
)


find_library(READLINE_LIB readline REQUIRED)

target_link_libraries(CLIDriver PRIVATE ${READLINE_LIB})