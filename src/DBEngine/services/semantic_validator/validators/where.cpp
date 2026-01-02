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
          LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_ColumnDoesNotExistInTable,
                                                           std::vector<std::string>{cond_col, table->first});
          return false;
        }
        // check if empty literal was given
        if (condition->literal_value->type == LiteralNode::Type::EMPTY) {
          LoggerService::ErrorLogger::printAsStandardError(
              StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_EmptyLiteralIsNotComparable);
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
                LoggerService::ErrorLogger::printAsStandardError(
                    StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorIsIsNotViolation);
                return false;
              } break;
            }

          } break;
          // [NOT] LIKE is only for strings
          case ComparatorNode::Type::LIKE:
          case ComparatorNode::Type::NOT_LIKE: {
            if (condition->literal_value->type != LiteralNode::Type::STRING) {
              LoggerService::ErrorLogger::printAsStandardError(
                  StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorLikeNotLikeViolation);
              return false;
            }
          } break;
          // EQ and NE are only allowed for strings and numbers
          case ComparatorNode::Type::EQ:
          case ComparatorNode::Type::NE: {
            if (condition->literal_value->type != LiteralNode::Type::NUMBER &&
                condition->literal_value->type != LiteralNode::Type::STRING) {
              LoggerService::ErrorLogger::printAsStandardError(
                  StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorEQNotEQViolation);
              return false;
            }
          } break;
          // default - mathematical comparator
          default: {
            if (condition->literal_value->type != LiteralNode::Type::NUMBER) {
              LoggerService::ErrorLogger::printAsStandardError(
                  StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorMathViolation);
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}