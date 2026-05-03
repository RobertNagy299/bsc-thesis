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

  if (node.opt_where_node) {
    // check if the given ID exists in the table for the specialized pk EQ case:
    const std::string& primary_key_column = Utilities::ColumnUtils::extractPrimaryKeyColumn(schema_cols);
    const std::string& pk_literal = node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value;
    const auto& pk_index = ctx.getHashmapIndices()->at(node.table_name)->at(primary_key_column);
    if (node.opt_where_node &&
        node.opt_where_node->conditions_list_node->conditions[0]->col_name == primary_key_column &&
        node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type == ComparatorNode::Type::EQ &&
        pk_index->find(pk_literal) == pk_index->end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_PrimaryKeyDoesNotExistInSpecialPKEQCase,
          std::vector<std::string>{pk_literal, node.table_name});
      return false;
    }
  }

  // ---- WHERE clause validation ----
  return SemanticValidator::validateWhereClauseSemantics(table_iterator, node.opt_where_node);
}
