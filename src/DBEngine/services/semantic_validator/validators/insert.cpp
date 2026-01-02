#include "../../../../auxiliary/types/types.hpp"
#include "../public_api.hpp"

/**
 * @param node <InsertNode>
 * @param ctx <ExecutionContext>
 *
 * @returns `true` if the insertion Node is semantically valid, `false` otherwise
 */
const bool SemanticValidator::validateInsertSemantics(InsertNode& node, const ExecutionContext& ctx) {
  auto untyped_tables = ctx.getUntypedTables();
  // use find instead of at because we do not know if it is actually present
  const auto& current_table = untyped_tables.find(node.table_name);
  if (current_table == untyped_tables.end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{node.table_name});
    return false;
  }
  const std::vector<ValueRecordNode*>& value_list = node.values->records;
  const std::vector<UntypedColumnDefNode*>& table_columns = current_table->second;
  const std::size_t value_record_length = value_list.size();
  const std::size_t table_cols_length = table_columns.size();
  std::cout << "Number of value records = " << std::to_string(value_record_length) << '\n';
  // node.columns is optional - if does not exist, is nullptr
  if (!node.columns) {
    // if there are empty values, make sure the omited values either have a default value
    // or are nullable, otherwise throw an error.
    for (std::size_t i = 0; i < value_record_length; ++i) {
      const std::vector<LiteralNode*>& literal_nodes_list = value_list.at(i)->values;
      const std::size_t current_literal_values_length = literal_nodes_list.size();
      // if there are more value nodes in a value record than in the table, throw an error
      if (current_literal_values_length > table_cols_length) {
        LoggerService::ErrorLogger::printAsStandardError(
            StatusCode::ErrorCode::SEMVAL_INSERT_MoreValuesThanColumnsInTable,
            std::vector<std::string>{node.table_name, std::to_string(table_cols_length),
                                     std::to_string(current_literal_values_length)});
        return false;
      }
      for (std::size_t j = 0; j < table_cols_length; ++j) {
        const std::vector<std::string>& current_modifiers = table_columns.at(j)->modifiers;
        const DB_Types::colmodifiers_t modifiers_checklist = Utilities::InsertUtils::getModifiers(current_modifiers);
        // TODO check for primary key and unique constraint violations after implementing indeces
        // check the provided literal nodes in the value record
        // otherwise, use defaults
        if (j < current_literal_values_length) {
          if (literal_nodes_list.at(j)->type == LiteralNode::Type::EMPTY) {
            if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(modifiers_checklist)) { return false; }
          }
        } else {
          if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(modifiers_checklist)) { return false; }
        }
      }
    }
    // no need to check if cols exist because none were given
    return true;
  } else {
    // In this case, the column list exists - validate null constaints
    const auto col_list_length = node.columns->columns.size();
    // Check if we have given more column names than there are columns in the table
    if (col_list_length > table_cols_length) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::SEMVAL_INSERT_MoreColumnsInColListThanInTable,
          std::vector<std::string>{node.table_name, std::to_string(table_cols_length),
                                   std::to_string(col_list_length)});
      return false;
    }
    // colname - modifiers map
    std::unordered_map<std::string, DB_Types::colmodifiers_t> col_modifiers;
    // Check if the specified columns exist in the table and get their modifiers
    for (const auto& col_name : node.columns->columns) {
      auto it = std::find_if(table_columns.begin(), table_columns.end(),
                             [&col_name](auto& col_node) { return col_node->name == col_name; });
      if (it == table_columns.end()) {
        LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_INSERT_ColumnDoesNotExistInTable,
                                                         std::vector<std::string>{col_name, node.table_name});
        return false;
      }

      const std::vector<std::string>& current_modifiers = (*it)->modifiers;
      const DB_Types::colmodifiers_t modifiers_checklist = Utilities::InsertUtils::getModifiers(current_modifiers);
      col_modifiers[col_name] = modifiers_checklist;
    }
    // iterate through the literal record and search for empty literals
    for (std::size_t i = 0; i < value_record_length; ++i) {
      // lazy eval, only get literal nodes list when we are sure there are no errors so far (hence this part is 'code
      // duplication')
      const std::vector<LiteralNode*>& literal_nodes_list = value_list.at(i)->values;
      const std::size_t current_literal_values_length = literal_nodes_list.size();
      // if there are more value nodes in a value record than in the table, throw an error
      if (current_literal_values_length > table_cols_length) {
        LoggerService::ErrorLogger::printAsStandardError(
            StatusCode::ErrorCode::SEMVAL_INSERT_MoreValuesThanColumnsInTable,
            std::vector<std::string>{node.table_name, std::to_string(table_cols_length),
                                     std::to_string(current_literal_values_length)});
        return false;
      }
      // First case: if there are more value nodes in the value record than columns in the col vector then the
      // statement is invalid
      if (current_literal_values_length > col_list_length) {
        LoggerService::ErrorLogger::printAsStandardError(
            StatusCode::ErrorCode::SEMVAL_INSERT_MoreValuesThanColumnsInColList,
            std::vector<std::string>{std::to_string(col_list_length), std::to_string(current_literal_values_length)});
        return false;
      }
      /****
       * Second case: column list specifies a column with the 'NOT NULL' / 'PK' constraint
       * but the value record does not provide a value there
       * */

      // Check explicit and implicit empty literals in a single for loop
      for (std::size_t j = 0; j < col_list_length; ++j) {
        const std::string current_colname = node.columns->columns.at(j);
        const auto& current_modifier_checklist = col_modifiers.at(current_colname);
        if (j >= current_literal_values_length) {
          // at this point, we know we have an implicit empty literal
          if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(current_modifier_checklist)) { return false; }
          continue;
        }
        // here, we have to deal with an explicit empty literal
        const auto& current_literal_node = literal_nodes_list.at(j);
        if (current_literal_node->type == LiteralNode::Type::EMPTY) {
          if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(current_modifier_checklist)) { return false; }
        }
      }
    }
  }

  return Utilities::ColumnUtils::columnsExistInTable(node.columns, current_table);
}