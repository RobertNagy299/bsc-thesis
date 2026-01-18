#pragma once
#include "../../../auxiliary/utils_public_api.hpp"
#include <string>

namespace ConditionEvaluator {

enum class RuntimeType { NULLTYPE, BOOLEAN, NUMBER, STRING };
RuntimeType classify(const std::string& s);
bool isNumeric(const std::string& s);
bool likeMatch(const std::string& text, const std::string& pattern);
bool evaluateComparator(const std::string& lhs, ComparatorNode::Type cmp, LiteralNode* rhs_node);

} // namespace ConditionEvaluator