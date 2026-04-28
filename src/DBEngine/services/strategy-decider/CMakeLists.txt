add_library(StrategyDecider SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/src/unique-equality-lookup.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/public-api.hpp
)

# target_link_libraries(StrategyDecider Utils)
