#include "../../../../auxiliary/types/types.hpp"
#include "../public_api.hpp"
#include <assert.h>

/**
 * @param node <InsertNode>
 * @param ctx <ExecutionContext>
 *
 * @returns `true` if the insertion Node is semantically valid, `false` otherwise
 */
const bool SemanticValidator::validateInsertSemantics(InsertNode& node, const ExecutionContext& ctx) {
  assert(node.is_normalized);

  const auto& tables = ctx.getUntypedTables();

  // table's existence was verified in normalizer
  const auto& schema_cols = tables.at(node.table_name);
  const std::size_t ncols = schema_cols.size();

  for (auto& record : node.values->records) {
    const auto& vals = record->values;

    // already guaranteed same length as schema
    for (std::size_t j = 0; j < ncols; ++j) {

      const auto& modifiers = Utilities::InsertUtils::getModifiers(schema_cols[j]->modifiers);

      const auto& lit = vals[j];

      const bool is_written = node.projection_mask[j];
      const bool is_empty = !lit || lit->type == LiteralNode::Type::EMPTY;

      if ((!is_written || is_empty) && Utilities::InsertUtils::hasEmptyLiteralRuleViolations(modifiers)) {
        return false;
      }
    }
  }

  return true;
}
