#include "../public_api.hpp"

const bool SemanticValidator::validateSelectSemantics(SelectNode& node, const ExecutionContext& ctx) {
  auto untyped_tables = ctx.getUntypedTables();
  const auto& current_table = untyped_tables.find(node.table_name);
  if (current_table == untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>({node.table_name}));
    return false;
  }

  // If no columns were given, then the ASTERISK was given, which is valid.
  // We only need to check if columns actually exist in the table if the user provides a col list
  if (node.columns) {
    if (!Utilities::ColumnUtils::columnsExistInTable(node.columns, current_table)) { return false; };
  }

  return SemanticValidator::validateWhereClauseSemantics(current_table, node.opt_where_node);
}