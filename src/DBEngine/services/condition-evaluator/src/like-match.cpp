#include "../public-api.hpp"

bool ConditionEvaluator::likeMatch(const std::string& text, const std::string& pattern) {
  // only support % wildcard
  if (pattern == "%") return true;

  auto pos = pattern.find('%');
  if (pos == std::string::npos) return text == pattern;

  std::string prefix = pattern.substr(0, pos);
  std::string suffix = pattern.substr(pos + 1);

  return Utilities::StringUtils::startsWith(text, prefix) && Utilities::StringUtils::hasSuffix(text, suffix);
}
