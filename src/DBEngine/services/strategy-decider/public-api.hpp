#pragma once
#include "../../../parser/ast.hpp"
#include "../../execution_context/public_api.hpp"
namespace StrategyDecider {
[[nodiscard]] const bool isUniqueEqualityLookup(const WhereNode* const& where_node, const std::string& unique_col_name);
} // namespace StrategyDecider
