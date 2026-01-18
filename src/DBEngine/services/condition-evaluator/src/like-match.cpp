#include "../public-api.hpp"
#include <regex>

bool ConditionEvaluator::likeMatch(const std::string& text, const std::string& pattern) {
  std::string regex_pattern;
  regex_pattern.reserve(pattern.size() * 2);

  for (char c : pattern) {
    switch (c) {
      case '%':
        regex_pattern += ".*";
        break;
      case '_':
        regex_pattern += ".";
        break;
      case '.':
      case '^':
      case '$':
      case '|':
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case '*':
      case '+':
      case '?':
      case '\\':
        regex_pattern += '\\';
        regex_pattern += c;
        break;
      default:
        regex_pattern += c;
    }
  }

  // match the entire string
  regex_pattern = "^" + regex_pattern + "$";

  try {
    std::regex re(regex_pattern, std::regex::ECMAScript);
    return std::regex_match(text, re);
  } catch (const std::regex_error&) {
    // defensive fallback: invalid pattern should not crash SELECT
    return false;
  }
}
