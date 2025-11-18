#include "../public_api.hpp"

const bool SemanticValidator::validateSelectSemantics(SelectNode& node, ExecutionContext& ctx) {
  const auto& current_table = ctx.untyped_tables.find(node.tableName);
  if (current_table == ctx.untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError("Table " + node.tableName + " does not exist.");
    return false;
  }

  // If no columns were given, then the ASTERISK was given, which is valid.
  // We only need to check if columns actually exist in the table if the user provides a col list
  if (node.columns) {
    if (!Utilities::ColumnUtils::columnsExistInTable(node.columns, current_table)) { return false; };
  }

  if (node.opt_where_node) {
    // TODO
  }

  return true;
}