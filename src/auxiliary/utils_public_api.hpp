#pragma once
#include "../DBEngine/services/logger/public_api.hpp"
#include "../parser/ast.hpp"
#include "types/types.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Utilities {

// List all the utils submodules
namespace ColumnUtils {

  const bool columnsExistInTable(ColumnListNode*& node, const DB_Types::untyped_table_t::const_iterator& table);
  const bool columnsExistInTable(const std::vector<UntypedColumnDefNode*>& columns, const std::string& col_name);
  const std::string& extractPrimaryKeyColumn(const std::vector<UntypedColumnDefNode*>& columns);
  std::unique_ptr<std::unordered_set<std::string>>
  extractColumnNamesFromTable(const DB_Types::untyped_table_t::const_iterator& table);
}; // namespace ColumnUtils

namespace StringUtils {

  std::string trim(const std::string& str);
  std::vector<std::string> splitString(const std::string& s, const std::string& delimiter);
  std::string removeOuterQuotes(std::string str);
  bool startsWith(const std::string& mainString, const std::string& prefixPattern);
  bool hasSuffix(const std::string& str, const std::string& suffix);
}; // namespace StringUtils

namespace InsertUtils {

  const DB_Types::colmodifiers_t getModifiers(const std::vector<std::string>& current_modifiers);
  const bool hasEmptyLiteralRuleViolations(const DB_Types::colmodifiers_t& modifiers_checklist);
  const std::string getDefaultValue(const std::vector<std::string>& current_modifiers);
}; // namespace InsertUtils
}; // namespace Utilities
