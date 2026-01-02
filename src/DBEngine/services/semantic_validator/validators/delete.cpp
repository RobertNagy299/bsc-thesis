#include "../../../../auxiliary/types/types.hpp"
#include "../public_api.hpp"

const bool SemanticValidator::validateDeleteSemantics(DeleteNode& node, const ExecutionContext& ctx) {
  // check if table exists
  const auto& untyped_tables = ctx.getUntypedTables();
  const auto& current_table = untyped_tables.find(node.table_name);
  if (current_table == untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{node.table_name});
    return false;
  }
  // Table exists, validate the Where clause
  return SemanticValidator::validateWhereClauseSemantics(current_table, node.opt_where_node);
}