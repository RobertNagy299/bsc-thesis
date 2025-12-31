#pragma once
#include "../DBEngine/services/logger/public_api.hpp"
#include "../parser/ast.hpp"
#include "types/types.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Utilities {

  // Delete the default constructor to prevent instantiation
  Utilities() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  Utilities(const Utilities&) = delete;
  Utilities& operator=(const Utilities&) = delete;

  // List all the utils submodules
  struct ColumnUtils {
    // Delete the default constructor to prevent instantiation
    ColumnUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    ColumnUtils(const ColumnUtils&) = delete;
    ColumnUtils& operator=(const ColumnUtils&) = delete;

    static const bool columnsExistInTable(ColumnListNode*& node, const untyped_table_t::const_iterator& table);
    static const std::string& extractPrimaryKeyColumn(const std::vector<UntypedColumnDefNode*>& columns);
    static std::unique_ptr<std::unordered_set<std::string>>
    extractColumnNamesFromTable(const untyped_table_t::const_iterator& table);
  };

  struct StringUtils {
    // Delete the default constructor to prevent instantiation
    StringUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    StringUtils(const StringUtils&) = delete;
    StringUtils& operator=(const StringUtils&) = delete;

    static std::string trim(const std::string& str);
    static std::vector<std::string> splitString(const std::string& s, const std::string& delimiter);
    static std::string removeOuterQuotes(std::string str);
  };

  struct InsertUtils {
    // Delete the default constructor to prevent instantiation
    InsertUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    InsertUtils(const InsertUtils&) = delete;
    InsertUtils& operator=(const InsertUtils&) = delete;

    static const colmodifiers_t getModifiers(const std::vector<std::string>& current_modifiers);
    static const bool hasEmptyLiteralRuleViolations(const colmodifiers_t& modifiers_checklist);
    static const std::string getDefaultValue(const std::vector<std::string>& current_modifiers);
  };
};
