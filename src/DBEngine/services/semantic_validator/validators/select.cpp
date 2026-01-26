#include "../public_api.hpp"

const bool SemanticValidator::validateSelectSemantics(SelectNode& node, const ExecutionContext& ctx) {
  const auto& tables = ctx.getUntypedTables();
  const auto& table_iterator = tables.find(node.table_name);
  if (!SemanticValidator::checkIfTableExists(node.table_name, ctx)) { return false; }

  const auto& schema_cols = table_iterator->second;

  // ---- Column list validation (if provided) ----
  if (node.columns) {
    if (!Utilities::ColumnUtils::columnsExistInTable(node.columns, table_iterator)) return false;
  }

  // ---- WHERE clause validation ----
  return SemanticValidator::validateWhereClauseSemantics(table_iterator, node.opt_where_node);
}
