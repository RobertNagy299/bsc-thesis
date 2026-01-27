#include "../public-api.hpp"

bool ConditionEvaluator::evaluateComparator(const std::string& lhs, ComparatorNode::Type cmp, LiteralNode* rhs_node) {

  const std::string& rhs = Utilities::StringUtils::removeOuterQuotes(rhs_node->value);

  RuntimeType ltype = classify(lhs);
  RuntimeType rtype = classify(rhs);

  switch (cmp) {

    case ComparatorNode::Type::IS: {
      // first check if this is a boolean comparison
      if (ltype == RuntimeType::BOOLEAN && rtype == RuntimeType::BOOLEAN) { return rhs == lhs; }
      // if it's not a boolean check, then it's a NULL check
      return ltype == RuntimeType::NULLTYPE;
    }

    case ComparatorNode::Type::IS_NOT: {
      // first check if it's a boolean
      if (ltype == RuntimeType::BOOLEAN && rtype == RuntimeType::BOOLEAN) { return rhs != lhs; }
      // if it's not a boolean, then it's a NULL check
      return ltype != RuntimeType::NULLTYPE;
    }

    case ComparatorNode::Type::LIKE:
      if (ltype != RuntimeType::STRING) return false;
      return likeMatch(lhs, rhs);

    case ComparatorNode::Type::NOT_LIKE:
      if (ltype != RuntimeType::STRING) return false;
      return !likeMatch(lhs, rhs);

    case ComparatorNode::Type::EQ:
      if (ltype != rtype) return false;
      return lhs == rhs;

    case ComparatorNode::Type::NE:
      if (ltype != rtype) return true;
      return lhs != rhs;

    case ComparatorNode::Type::LT:
    case ComparatorNode::Type::LE:
    case ComparatorNode::Type::GT:
    case ComparatorNode::Type::GE:
      if (ltype != RuntimeType::NUMBER || rtype != RuntimeType::NUMBER) return false;

      double l = std::stod(lhs);
      double r = std::stod(rhs);

      if (cmp == ComparatorNode::Type::LT) return l < r;
      if (cmp == ComparatorNode::Type::LE) return l <= r;
      if (cmp == ComparatorNode::Type::GT) return l > r;
      if (cmp == ComparatorNode::Type::GE) return l >= r;

      return false;
  }

  return false;
}
