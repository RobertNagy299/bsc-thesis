add_library(FileHandler SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/handler-utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/create_untyped.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/drop.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/insert.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/serialization/serialize-record.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)