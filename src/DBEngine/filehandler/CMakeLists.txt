add_library(FileHandler SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/handler-utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/create_untyped.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/drop.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/insert.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/select.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/delete.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/update.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/handlers/csv-import.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/serialization/normalized-record.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/serialization/create-new-tablefile.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/deserialization/extract-pks.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/deserialization/record.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/deserialization/record-type.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/compactor/ratio-calc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/compactor/compact.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/compactor/copy-record.cpp



  ${CMAKE_CURRENT_SOURCE_DIR}/public_api.hpp
)

target_link_libraries(FileHandler ConditionEvaluator)