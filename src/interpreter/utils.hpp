#pragma once
#include <vector>
#include <string>
#include <iostream>

struct ColModifierChecklist
{
  bool has_default : 1;
  bool primary_key : 1;
  bool not_null : 1;
};

// Function to trim leading and trailing whitespace
std::string trim(const std::string &str)
{
  const std::string whitespace = " \t\n\r\f\v"; // Common whitespace characters

  // Find the first non-whitespace character
  size_t first_non_ws = str.find_first_not_of(whitespace);
  if (std::string::npos == first_non_ws)
  {
    return ""; // Entire string is whitespace
  }

  // Find the last non-whitespace character
  size_t last_non_ws = str.find_last_not_of(whitespace);

  // Extract the substring
  return str.substr(first_non_ws, (last_non_ws - first_non_ws + 1));
}

std::vector<std::string> splitString(const std::string &s, const std::string &delimiter)
{
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = s.find(delimiter);

  while (end != std::string::npos)
  {
    tokens.push_back(s.substr(start, end - start));
    start = end + delimiter.length();
    end = s.find(delimiter, start);
  }
  tokens.push_back(s.substr(start)); // Add the last token

  return tokens;
}

const ColModifierChecklist getModifiers(const std::vector<std::string> &current_modifiers)
{
  ColModifierChecklist answer = ColModifierChecklist();
  for (size_t k = 0; k < current_modifiers.size(); ++k)
  {
    const std::string &current_modifier = trim(current_modifiers.at(k));
    if (current_modifier.find("DEFAULT") != std::string::npos)
    {
      answer.has_default = 1;
    }
    if (current_modifier == "PRIMARY KEY")
    {
      answer.primary_key = 1;
    }
    if (current_modifier == "NOT NULL")
    {
      answer.not_null = 1;
    }
  }
  return answer;
}

const bool hasEmptyLiteralRuleViolations(const ColModifierChecklist &modifiers_checklist)
{
  if (modifiers_checklist.primary_key)
  {
    std::cerr << "Error (code INSRT-0003) - Primary Key cannot be empty value.\n";
    return true;
  }
  if (modifiers_checklist.not_null && !modifiers_checklist.has_default)
  {
    std::cerr << "Error (code: INSRT-0002) - cannot insert empty literal into column marked as NOT NULL without explicit DEFAULT value\n";
    return true;
  }
  return false;
}