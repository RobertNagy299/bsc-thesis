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
  if (node.opt_where_node) {
    // check if the given ID exists in the table for the specialized pk EQ case:
    const std::string& primary_key_column = Utilities::ColumnUtils::extractPrimaryKeyColumn(current_table->second);
    const std::string& pk_literal = node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value;
    const auto& pk_index = ctx.getHashmapIndices()->at(node.table_name)->at(primary_key_column);
    if (node.opt_where_node &&
        node.opt_where_node->conditions_list_node->conditions[0]->col_name == primary_key_column &&
        node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type == ComparatorNode::Type::EQ &&
        pk_index->find(pk_literal) == pk_index->end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_DELETE_PrimaryKeyDoesNotExistInSpecialPKEQCase,
          std::vector<std::string>{pk_literal, node.table_name});
      return false;
    }
  }

  // Table exists, validate the Where clause
  return SemanticValidator::validateWhereClauseSemantics(current_table, node.opt_where_node);
}