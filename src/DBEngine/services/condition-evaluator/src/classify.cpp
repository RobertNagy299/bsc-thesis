#include "../public-api.hpp"

ConditionEvaluator::RuntimeType ConditionEvaluator::classify(const std::string& s) {
  if (s.empty() || s == "NULL") return RuntimeType::NULLTYPE;
  if (s == "TRUE" || s == "FALSE") return RuntimeType::BOOLEAN;
  if (isNumeric(s)) return RuntimeType::NUMBER;
  return RuntimeType::STRING;
}