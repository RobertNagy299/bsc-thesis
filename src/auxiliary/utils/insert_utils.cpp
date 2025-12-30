#include "../types/types.hpp"
#include "../utils_public_api.hpp"
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief
 * This class is intended to be a `static` method container, do not instantiate it!
 */

const colmodifiers_t Utilities::InsertUtils::getModifiers(const std::vector<std::string>& current_modifiers) {
  auto answer = colmodifiers_t();
  for (size_t k = 0; k < current_modifiers.size(); ++k) {
    const std::string& current_modifier = Utilities::StringUtils::trim(current_modifiers.at(k));
    if (current_modifier.find("DEFAULT") != std::string::npos) {
      answer.has_default = 1;
      continue;
    }
    if (current_modifier == "PRIMARY KEY") {
      answer.primary_key = 1;
      continue;
    }
    if (current_modifier == "NOT NULL") {
      answer.not_null = 1;
      continue;
    }
    if (current_modifier == "UNIQUE") {
      answer.unique = 1;
      continue;
    }
  }
  return answer;
}

/**
 * @brief this function is called when we already know that the current literal is empty
 * @param modifiers_checklist > colmodifiers_t
 *
 * @returns `true` if empty literal rule violations were found.
 *  */
const bool Utilities::InsertUtils::hasEmptyLiteralRuleViolations(const colmodifiers_t& modifiers_checklist) {
  if (modifiers_checklist.primary_key) {
    LoggerService::ErrorLogger::printAsStandardError("Error (Code: INSRT-0003) - Primary Key cannot be empty value.\n" +
                                                     std::string(" coming from file : ") + std::string(__FILE__) +
                                                     " Line : #" + std::to_string(__LINE__));
    return true;
  }
  if (modifiers_checklist.not_null && !modifiers_checklist.has_default) {
    LoggerService::ErrorLogger::printAsStandardError(
        "Error (Code: INSRT-0002) - cannot insert empty literal into column marked as NOT NULL without " +
        std::string("explicit DEFAULT value\n") + "\n coming from file : " + std::string(__FILE__) + " Line : #" +
        std::to_string(__LINE__));
    return true;
  }
  return false;
}

const std::string Utilities::InsertUtils::getDefaultValue(const std::vector<std::string>& current_modifiers) {
  for (size_t k = 0; k < current_modifiers.size(); ++k) {
    const std::string& current_modifier = Utilities::StringUtils::trim(current_modifiers.at(k));
    if (current_modifier.find("DEFAULT") != std::string::npos) {
      return Utilities::StringUtils::removeOuterQuotes(current_modifier.substr(8));
    }
  }
}