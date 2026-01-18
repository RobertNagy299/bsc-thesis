#include "../public-api.hpp"

bool ConditionEvaluator::isNumeric(const std::string& s) {
  char* end = nullptr;
  std::strtod(s.c_str(), &end);
  return end != s.c_str() && *end == '\0';
}