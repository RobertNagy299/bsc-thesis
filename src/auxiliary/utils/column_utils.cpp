#include "../../parser/ast.hpp"
#include "../utils_public_api.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

const bool Utilities::ColumnUtils::columnsExistInTable(
    ColumnListNode*& node, const std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>>::iterator& table) {

  // use a hash map for faster lookups
  std::unordered_set<std::string> auxiliary_colname_hashmap;
  const auto table_cols = table->second;
  // put the table's columns into the hashmap
  for (const auto& col_node : table_cols) {
    const std::string& col_name = col_node->name;
    auxiliary_colname_hashmap.insert(col_name);
  }
  // check if the given cols exist
  for (const auto& col_name : node->columns) {
    // if col is not found, then the statement is invalid
    if (auxiliary_colname_hashmap.find(col_name) == auxiliary_colname_hashmap.end()) {
      LoggerService::ErrorLogger::printAsStandardError(
          "Error (Code: COLUMN-VALIDATOR-0001): Column " + col_name + " does not exist in table " + table->first + '!' +
          "\n coming from file : " + std::string(__FILE__) + " Line : #" + std::to_string(__LINE__));
      return false;
    }
  }
  return true;
}
