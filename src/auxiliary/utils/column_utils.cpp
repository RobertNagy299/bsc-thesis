#include "../../parser/ast.hpp"
#include "../types/types.hpp"
#include "../utils_public_api.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

std::unique_ptr<std::unordered_set<std::string>>
Utilities::ColumnUtils::extractColumnNamesFromTable(const DB_Types::untyped_table_t::const_iterator& table) {
  // use a hash map for faster lookups
  auto auxiliary_colname_hashmap = std::make_unique<std::unordered_set<std::string>>();
  const auto table_cols = table->second;
  // put the table's columns into the hashmap
  for (const auto& col_node : table_cols) {
    const std::string& col_name = col_node->name;
    auxiliary_colname_hashmap->insert(col_name);
  }
  return std::move(auxiliary_colname_hashmap);
}

const bool Utilities::ColumnUtils::columnsExistInTable(ColumnListNode*& node,
                                                       const DB_Types::untyped_table_t::const_iterator& table) {

  auto auxiliary_colname_hashmap = Utilities::ColumnUtils::extractColumnNamesFromTable(table);
  // check if the given cols exist
  for (const auto& col_name : node->columns) {
    // if col is not found, then the statement is invalid
    if (auxiliary_colname_hashmap->find(col_name) == auxiliary_colname_hashmap->end()) {
      // clang-format off
      LoggerService::ErrorLogger::printAsStandardError(
          "Error (Code: COLUMN-VALIDATOR-0001): Column " + col_name + " does not exist in table " + table->first + '!' +
          "\n coming from file : " + std::string(__FILE__) + " Line : #" + std::to_string(__LINE__));
      // clang-format on 
      return false;
    }
  }
  return true;
}

const std::string& Utilities::ColumnUtils::extractPrimaryKeyColumn(const std::vector<UntypedColumnDefNode*> &columns){
  for(const auto& col : columns) {
    if(Utilities::InsertUtils::getModifiers(col->modifiers).primary_key) {
      return col->name;
    }
  }
  // TODO account for tables without a primary key
}