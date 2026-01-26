#include "../public_api.hpp"

const bool SemanticValidator::validateDropSemantics(const DropTableNode& node, const ExecutionContext& ctx) {
  return SemanticValidator::checkIfTableExists(node.table_name, ctx);
}
