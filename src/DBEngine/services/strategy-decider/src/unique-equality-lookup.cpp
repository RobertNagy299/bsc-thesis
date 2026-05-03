#include "../public-api.hpp"

[[nodiscard]] const bool StrategyDecider::isUniqueEqualityLookup(const WhereNode* const& opt_where_node,
                                                                 const std::string& unique_col_name) {
  if (opt_where_node && opt_where_node->conditions_list_node->conditions[0]->col_name == unique_col_name &&
      opt_where_node->conditions_list_node->conditions[0]->cmp_node->type == ComparatorNode::Type::EQ) {
    return true;
  }
  return false;
}