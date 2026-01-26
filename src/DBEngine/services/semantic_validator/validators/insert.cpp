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
  if (!SemanticValidator::checkIfTableExists(node.table_name, ctx)) { return false; }

  const auto& tables = ctx.getUntypedTables();
  const std::string& table_pk_col = Utilities::ColumnUtils::extractPrimaryKeyColumn(tables.at(node.table_name));

  const auto& pk_hashmap_index = ctx.getHashmapIndices()->at(node.table_name)->at(table_pk_col);
  std::unordered_set<std::string> seen_keys_in_input;
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
      // check primary key violation
      if (!is_empty && modifiers.primary_key) {
        if ((seen_keys_in_input.find(lit->value) != seen_keys_in_input.end()) ||
            (pk_hashmap_index->find(lit->value) != pk_hashmap_index->end())) {
          // if it is found, we have a duplicate
          LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_INSERT_DuplicatePrimaryKeys,
                                                           std::vector<std::string>{lit->value, node.table_name});
          return false;
        }
        seen_keys_in_input.insert(lit->value);
      }
    }
  }

  return true;
}
