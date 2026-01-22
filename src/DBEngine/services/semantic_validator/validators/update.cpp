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

  std::unordered_set<std::string> assignment_cols;
  const std::string& primary_key_column = Utilities::ColumnUtils::extractPrimaryKeyColumn(current_table->second);
  // check columns
  auto table_cols = Utilities::ColumnUtils::extractColumnNamesFromTable(current_table);
  for (const auto& assignment_node : node.assignment_list_node->assignments) {
    if (table_cols->find(assignment_node->col_name) == table_cols->end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_ColumnDoesNotExistInTable,
          std::vector<std::string>{assignment_node->col_name, current_table->first});
      return false;
    }
    // check if the user is trying to modify the same column in a single command
    if (assignment_cols.find(assignment_node->col_name) != assignment_cols.end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_UPDATE_TriedUpdatingTheSameColumnInOneCommand,
          std::vector<std::string>{assignment_node->col_name});
      return false;
    }
    assignment_cols.insert(assignment_node->col_name);

    // check if we are trying to update the primary key - which is a bad practice
    if (primary_key_column == assignment_node->col_name) {
      LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_UPDATE_TriedUpdatingThePrimaryKey,
                                                       std::vector<std::string>{primary_key_column});
      return false;
    }
  }

  // validate the Where clause
  return SemanticValidator::validateWhereClauseSemantics(current_table, node.opt_where_node);
}