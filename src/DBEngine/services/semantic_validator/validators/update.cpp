#include "../public_api.hpp"

const bool SemanticValidator::validateUpdateSemantics(UpdateNode& node, const ExecutionContext& ctx) {
  // check if table exists
  const auto& untyped_tables = ctx.getUntypedTables();
  const auto& current_table = untyped_tables.find(node.table_name);
  if (current_table == untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{node.table_name});
    return false;
  }

  // check columns
  auto table_cols = Utilities::ColumnUtils::extractColumnNamesFromTable(current_table);
  for (const auto& assignment_node : node.assignment_list_node->assignments) {
    if (table_cols->find(assignment_node->col_name) == table_cols->end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_ColumnDoesNotExistInTable,
          std::vector<std::string>{assignment_node->col_name, current_table->first});
      return false;
    }
  }

  // Table exists, validate the Where clause
  return SemanticValidator::validateWhereClauseSemantics(current_table, node.opt_where_node);
}