#include "../public_api.hpp"

const bool SemanticValidator::validateSelectSemantics(SelectNode& node, const ExecutionContext& ctx) {
  auto& tables = ctx.getUntypedTables();
  auto t = tables.find(node.table_name);
  if (t == tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{node.table_name});
    return false;
  }

  const auto& schema_cols = t->second;

  // ---- Column list validation (if provided) ----
  if (node.columns) {
    if (!Utilities::ColumnUtils::columnsExistInTable(node.columns, t)) return false;
  }

  // ---- WHERE clause validation ----
  return SemanticValidator::validateWhereClauseSemantics(t, node.opt_where_node);
}
