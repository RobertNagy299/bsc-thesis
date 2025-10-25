#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../parser/ast.hpp"
#include "execution_context.hpp"
#include "utils.hpp"

/**
 * @brief
 * This class is intended to be a `static` method container, do not instantiate it!
 */
struct SemanticValidator
{
  // Delete the default constructor to prevent instantiation
  SemanticValidator() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  SemanticValidator(const SemanticValidator &) = delete;
  SemanticValidator &operator=(const SemanticValidator &) = delete;
  /**
   * @param node <InsertNode>
   * @param ctx <ExecutionContext>
   *
   * @returns `true` if the insertion Node is semantically valid, `false` otherwise
   */
  static const bool validateInsertSemantics(InsertNode &node, ExecutionContext &ctx)
  {
    const auto &current_table = ctx.untyped_tables.find(node.tableName);
    if (current_table == ctx.untyped_tables.end())
    {
      std::cerr << "Table " << node.tableName << " does not exist.";
      return false;
    }

    // node.columns is optional - if does not exist, is nullptr
    if (!node.columns)
    {
      const std::vector<ValueRecordNode *> &value_list = node.values->records;
      const std::vector<UntypedColumnDefNode *> &table_columns = current_table->second;
      const size_t value_record_length = value_list.size();
      const size_t table_cols_length = table_columns.size();

      // if there are empty values, make sure the omited values either have a default value
      // or are nullable, otherwise throw an error.
      try
      {
        for (size_t i = 0; i < value_record_length; ++i)
        {
          const std::vector<LiteralNode *> &literal_nodes_list = value_list.at(i)->values;
          const size_t current_literal_values_length = literal_nodes_list.size();
          // if there are more value nodes in a value record than in the table, throw an error
          if (current_literal_values_length > table_cols_length)
          {
            std::cerr << "Error (code: INSRT-0001): " << node.tableName << " Only has " << table_cols_length << " columns, but you tried to insert "
                      << current_literal_values_length << " values in a row.\n";
          }
          for (size_t j = 0; j < table_cols_length; ++j)
          {
            const std::vector<std::string> &current_modifiers = table_columns.at(j)->modifiers;
            const ColModifierChecklist &modifiers_checklist = Utilities::InsertUtils::getModifiers(current_modifiers);

            // check the provided literal nodes in the value record
            // otherwise, use defaults
            if (j < current_literal_values_length)
            {
              if (literal_nodes_list.at(j)->type == LiteralNode::Type::EMPTY)
              {
                if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(modifiers_checklist))
                {
                  return false;
                }
              }
            }
            else
            {
              if (Utilities::InsertUtils::hasEmptyLiteralRuleViolations(modifiers_checklist))
              {
                return false;
              }
            }
          }
        }

        return true;
      }
      catch (const std::exception &e)
      {
        std::cerr << "Error during value insertion: " << e.what() << '\n';
      }
    }
  }
};
