#include "../public-api.hpp"

bool SemanticNormalizer::normalizeDelete(DeleteNode& node, const ExecutionContext& ctx) {
  auto it = ctx.getUntypedTables().find(node.table_name);
  if (it == ctx.getUntypedTables().end()) return false;

  if (node.opt_where_node) {
    node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value =
        Utilities::StringUtils::removeOuterQuotes(
            node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value);
    return normalizeWhereClause(node.opt_where_node, it->second);
  }

  return true;
}
