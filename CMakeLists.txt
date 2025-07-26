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
add_library(DBEngine SHARED src/DBEngine/engine.cpp)

# Tell the compiler to look in the binary dir for the generated header
include_directories(${CMAKE_BINARY_DIR})

# Include headers
target_include_directories(
  DBEngine 
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser     # <- to find ast.hpp
    ${CMAKE_CURRENT_BINARY_DIR}                # <- to find parser.hpp (generated)   
  )

# Create the main executable
add_executable(MainApp 
  src/main.cpp
  src/parser/ast.cpp
  ${BISON_Parser_OUTPUTS}
  ${FLEX_Lexer_OUTPUTS}  
)

# Link the executable with the shared lib
target_link_libraries(MainApp DBEngine)
