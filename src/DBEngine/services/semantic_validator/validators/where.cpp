#include "../../../../auxiliary/types/types.hpp"
#include "../public_api.hpp"

const bool SemanticValidator::validateWhereClauseSemantics(const DB_Types::untyped_table_t::const_iterator& table,
                                                           WhereNode* where_node) {

  // Empty where clause is correct
  if (!where_node) { return true; }

  auto colnames = Utilities::ColumnUtils::extractColumnNamesFromTable(table);

  // Iterate through each condition and check the validity of each condition
  if (where_node->conditions_list_node) {
    for (const auto& condition : where_node->conditions_list_node->conditions) {
      if (condition) {
        // check if column exists
        const auto& cond_col = condition->col_name;
        if (colnames->find(cond_col) == colnames->end()) {
          // clang-format off
          LoggerService::ErrorLogger::printAsStandardError("Error: (Code: WHERE-0001), column " + cond_col +
                                                           " Does not exist " + "in table " + table->first);

          // clang-format on
          return false;
        }
        // check if empty literal was given
        if (condition->literal_value->type == LiteralNode::Type::EMPTY) {
          // clang-format off
          LoggerService::ErrorLogger::printAsStandardError(
              "Error: (Code: WHERE-0002), literal in WHERE condition cannot be empty literal!");

          // clang-format on
          return false;
        }

        // verify comparator correctness
        switch (condition->cmp_node->type) {
          case ComparatorNode::Type::IS:
          case ComparatorNode::Type::IS_NOT: {
            switch (condition->literal_value->type) {
              case LiteralNode::Type::NUMBER:
              case LiteralNode::Type::STRING: {
                // IS [NOT] NULL / FALSE / TRUE
                // clang-format off
                LoggerService::ErrorLogger::printAsStandardError(
                    "Error: (Code: WHERE-0003), comparators 'IS' and 'IS NOT' must be followed by either a 'NULL' or a "
                    "boolean (TRUE or FALSE) "
                    "literal!");

                // clang-format on
                return false;
              } break;
            }

          } break;
          // [NOT] LIKE is only for strings
          case ComparatorNode::Type::LIKE:
          case ComparatorNode::Type::NOT_LIKE: {
            if (condition->literal_value->type != LiteralNode::Type::STRING) {
              // clang-format off
              LoggerService::ErrorLogger::printAsStandardError(
                  "Error: (Code: WHERE-0004), comparators 'LIKE' and 'NOT LIKE' must be followed by a string literal!");

              // clang-format on
              return false;
            }
          } break;
          // EQ and NE are only allowed for strings and numbers
          case ComparatorNode::Type::EQ:
          case ComparatorNode::Type::NE: {
            if (condition->literal_value->type != LiteralNode::Type::NUMBER &&
                condition->literal_value->type != LiteralNode::Type::STRING) {
              // clang-format off
              LoggerService::ErrorLogger::printAsStandardError("Error: (Code: WHERE-0005): comparators '=' and '!=' "
                                                               "can only be used with string or numeric literals");

              // clang-format on
              return false;
            }
          } break;
          // default - mathematical comparator
          default: {
            if (condition->literal_value->type != LiteralNode::Type::NUMBER) {
              // clang-format off
              LoggerService::ErrorLogger::printAsStandardError(
                  "Error: (Code: WHERE-0006), Mathematical comparators must be followed by a numeric literal!");

              // clang-format on
              return false;
            }
          }
        }
      }
    }
  }
  // clang-format off

  // clang-format on
  return true;
}