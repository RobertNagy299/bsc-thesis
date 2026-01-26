#include "../public_api.hpp"

const bool SemanticValidator::validateDescribeSemantics(const DescribeNode& node, const ExecutionContext& ctx) {
  return SemanticValidator::checkIfTableExists(node.table_name, ctx);
}
