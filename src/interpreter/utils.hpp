#pragma once
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

struct ColModifierChecklist {
  bool has_default : 1;
  bool primary_key : 1;
  bool not_null : 1;
};

/**
 * @brief
 * This class is intended to be a `static` method container, do not instantiate it!
 */
struct Utilities {

  // Delete the default constructor to prevent instantiation
  Utilities() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  Utilities(const Utilities&) = delete;
  Utilities& operator=(const Utilities&) = delete;

  /**
   * @brief
   * This class is intended to be a `static` method container, do not instantiate it!
   */
  struct ColumnUtils {
    // Delete the default constructor to prevent instantiation
    ColumnUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    ColumnUtils(const ColumnUtils&) = delete;
    ColumnUtils& operator=(const ColumnUtils&) = delete;

    static const bool
    columnsExistInTable(ColumnListNode*& node,
                        const std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>>::iterator& table) {
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
          std::cerr << std::endl
                    << "Error (Code: COLUMN-VALIDATOR-0001): Column " << col_name << " does not exist in table "
                    << table->first << '!'
                    << "\n coming from file : " + std::string(__FILE__) + " Line : #" + std::to_string(__LINE__)
                    << std::endl
                    << std::endl;
          return false;
        }
      }
      return true;
    }
  };

  /**
   * @brief
   * This class is intended to be a `static` method container, do not instantiate it!
   */
  struct StringUtils {
    // Delete the default constructor to prevent instantiation
    StringUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    StringUtils(const StringUtils&) = delete;
    StringUtils& operator=(const StringUtils&) = delete;

    // Function to trim leading and trailing whitespace
    static std::string trim(const std::string& str) {
      const std::string whitespace = " \t\n\r\f\v"; // Common whitespace characters

      // Find the first non-whitespace character
      size_t first_non_ws = str.find_first_not_of(whitespace);
      if (std::string::npos == first_non_ws) {
        return ""; // Entire string is whitespace
      }

      // Find the last non-whitespace character
      size_t last_non_ws = str.find_last_not_of(whitespace);

      // Extract the substring
      return str.substr(first_non_ws, (last_non_ws - first_non_ws + 1));
    }

    static std::vector<std::string> splitString(const std::string& s, const std::string& delimiter) {
      std::vector<std::string> tokens;
      size_t start = 0;
      size_t end = s.find(delimiter);

      while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
      }
      tokens.push_back(s.substr(start)); // Add the last token

      return tokens;
    }
  };

  /**
   * @brief
   * This class is intended to be a `static` method container, do not instantiate it!
   */
  struct InsertUtils {

    // Delete the default constructor to prevent instantiation
    InsertUtils() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    InsertUtils(const InsertUtils&) = delete;
    InsertUtils& operator=(const InsertUtils&) = delete;

    static const ColModifierChecklist getModifiers(const std::vector<std::string>& current_modifiers) {
      ColModifierChecklist answer = ColModifierChecklist();
      for (size_t k = 0; k < current_modifiers.size(); ++k) {
        const std::string& current_modifier = Utilities::StringUtils::trim(current_modifiers.at(k));
        if (current_modifier.find("DEFAULT") != std::string::npos) { answer.has_default = 1; }
        if (current_modifier == "PRIMARY KEY") { answer.primary_key = 1; }
        if (current_modifier == "NOT NULL") { answer.not_null = 1; }
      }
      return answer;
    }

    /**
     * @param modifiers_checklist > ColModifierChecklist
     *
     * @returns `true` if empty literal rule violations were found.
     *  */
    static const bool hasEmptyLiteralRuleViolations(const ColModifierChecklist& modifiers_checklist) {
      if (modifiers_checklist.primary_key) {
        std::cerr << "Error (Code: INSRT-0003) - Primary Key cannot be empty value.\n"
                  << "\n coming from file : " + std::string(__FILE__) + " Line : #" + std::to_string(__LINE__)
                  << std::endl;
        return true;
      }
      if (modifiers_checklist.not_null && !modifiers_checklist.has_default) {
        std::cerr << "Error (Code: INSRT-0002) - cannot insert empty literal into column marked as NOT NULL without "
                     "explicit DEFAULT value\n"
                  << "\n coming from file : " + std::string(__FILE__) + " Line : #" + std::to_string(__LINE__)
                  << std::endl;
        return true;
      }
      return false;
    }
  };
};
