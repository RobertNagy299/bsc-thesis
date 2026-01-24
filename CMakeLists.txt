cmake_minimum_required(VERSION 3.10)
project(JsonDatabaseWithSQL CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(
  Parser
  src/parser/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
)

flex_target(
  Lexer
  src/parser/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

add_flex_bison_dependency(Lexer Parser)

# Create the shared library
# Core library with parser + engine (no main.cpp!)
add_library(SQLCore SHARED
    src/parser/ast.cpp
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/filehandler
)

# Tell the compiler to look in the binary dir for the generated header
include_directories(${CMAKE_BINARY_DIR})

# Include headers
target_include_directories(
  SQLCore
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser     # <- to find ast.hpp
    ${CMAKE_CURRENT_BINARY_DIR}                # <- to find parser.hpp (generated)
    ${CMAKE_CURRENT_BINARY_DIR}/src/interpreter
  )

# Create the main executable
add_executable(MainApp 
  src/main.cpp
  src/parser/ast.cpp
  ${BISON_Parser_OUTPUTS}
  ${FLEX_Lexer_OUTPUTS}  
)

### Shared libs (.so / .dll)

# defines LoggerService
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/services/logger)

# defines Utils
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/auxiliary)

# defines ExecutionContext
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/execution_context)

# defines FileHandler
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/filehandler)

# defines SemanticValidator 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/services/semantic_validator)

# defines SemanticNormalizer
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/services/semantic_normalizer)

# defines ConditionEvaluator
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/DBEngine/services/condition-evaluator)

# defines CLIDriver
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/CLI-driver)

# Link the executable with the shared libraries
target_link_libraries(MainApp 
  SQLCore 
  Utils 
  ConditionEvaluator
  SemanticValidator 
  SemanticNormalizer
  LoggerService
  FileHandler
  ExecutionContext
  CLIDriver
)


# Enable tests
enable_testing()
add_subdirectory(tests)
