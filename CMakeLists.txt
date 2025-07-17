cmake_minimum_required(VERSION 3.10)
project(JsonDatabaseWithSQL CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the shared library
add_library(SQLParser SHARED src/parser/parser.cpp)

# Include headers
target_include_directories(SQLParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Create the main executable
add_executable(MainApp src/main.cpp)

# Link the executable with the shared lib
target_link_libraries(MainApp SQLParser)

# OLd working
# cmake_minimum_required(VERSION 3.10)
# project(JsonDatabaseWithSQL CXX)
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_library(libSQLParser SHARED src/parser/parser.cpp)
# add_executable(main src/main.cpp)
# target_link_libraries(main libSQLParser)