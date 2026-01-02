#include "../types/types.hpp"
#include "../utils_public_api.hpp"
#include <iostream>
#include <string>
#include <vector>

const DB_Types::colmodifiers_t Utilities::InsertUtils::getModifiers(const std::vector<std::string>& current_modifiers) {
  auto answer = DB_Types::colmodifiers_t();
  for (std::size_t k = 0; k < current_modifiers.size(); ++k) {
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
 * @param modifiers_checklist > DB_Types::colmodifiers_t
 *
 * @returns `true` if empty literal rule violations were found.
 *  */
const bool Utilities::InsertUtils::hasEmptyLiteralRuleViolations(const DB_Types::colmodifiers_t& modifiers_checklist) {
  if (modifiers_checklist.primary_key) {
    LoggerService::ErrorLogger::printAsStandardError(
        StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_PrimaryKeyCannotBeEmpty);
    return true;
  }
  if (modifiers_checklist.not_null && !modifiers_checklist.has_default) {
    LoggerService::ErrorLogger::printAsStandardError(
        StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_NotNullNoDefaultViolation);
    return true;
  }
  return false;
}

/**
 * @brief this function should only be called once we are 100% sure that the column has a default value, because the
 * control reaches the end of this non-void function and doesn't return anything if it can't find a default value. this
 * is actually by design - perhaps a poor design choice by me. - Author.
 */
const std::string Utilities::InsertUtils::getDefaultValue(const std::vector<std::string>& current_modifiers) {
  for (std::size_t k = 0; k < current_modifiers.size(); ++k) {
    const std::string& current_modifier = Utilities::StringUtils::trim(current_modifiers.at(k));
    if (current_modifier.find("DEFAULT") != std::string::npos) {
      return Utilities::StringUtils::removeOuterQuotes(current_modifier.substr(8));
    }
  }
}