add_library(ConditionEvaluator SHARED

  ${CMAKE_CURRENT_SOURCE_DIR}/src/classify.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/isnumeric.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/like-match.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/evaluate.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public-api.hpp
)

target_link_libraries(ConditionEvaluator Utils)
