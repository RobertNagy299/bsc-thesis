add_library(CLIDriver SHARED

  ${CMAKE_CURRENT_SOURCE_DIR}/src/runcli.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public-api.hpp
)

target_link_libraries(CLIDriver
  PRIVATE
    Readline::Readline
    SemanticValidator 
    SemanticNormalizer
    LoggerService
    FileHandler
    ExecutionContext
)