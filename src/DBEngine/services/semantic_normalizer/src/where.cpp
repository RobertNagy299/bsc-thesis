#include "../public-api.hpp"

bool SemanticNormalizer::normalizeWhereClause(WhereNode* where, const std::vector<UntypedColumnDefNode*>& schema_cols) {

  if (!where) return true; // nothing to normalize

  auto& conds = where->conditions_list_node->conditions;
  const std::size_t ncols = schema_cols.size();

  for (auto& cond : conds) {

    // --- resolve column name → schema index ---
    std::size_t idx = 0;
    for (; idx < ncols; ++idx)
      if (schema_cols[idx]->name == cond->col_name) break;

    if (idx == ncols) {
      // unknown column — validator will report
      return false;
    }

    cond->schema_index = idx;

    // Later you can also normalize literals / coercions here if desired
  }

  return true;
}
