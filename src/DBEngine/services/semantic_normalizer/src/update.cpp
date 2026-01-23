#include "../public-api.hpp"

bool SemanticNormalizer::normalizeUpdate(UpdateNode& node, const ExecutionContext& ctx) {
  // 1. Check table existence
  const auto& tables = ctx.getUntypedTables();
  auto table_it = tables.find(node.table_name);
  if (table_it == tables.end()) {
    // validator will log the error
    return false;
  }

  const auto& schema = table_it->second;

  // 2. Extract primary key column
  const std::string primary_key_col = Utilities::ColumnUtils::extractPrimaryKeyColumn(schema);
  // 3. Validate assignment list
  if (!node.assignment_list_node || node.assignment_list_node->assignments.empty()) {
    // validator will log - error, update must have an assignment
    return false;
  }
  std::unordered_set<std::string> assigned_columns;
  // mark every value as if it's being kept in its old form
  node.inverse_proj_mask.assign(schema.size(), true);

  // get canonical order of cols
  std::vector<std::string> full_col_list;
  full_col_list.reserve(schema.size());
  for (auto& col : schema) full_col_list.push_back(col->name);

  for (auto& assignment : node.assignment_list_node->assignments) {
    const std::string& col_name = assignment->col_name;
    // 3.5 Normalize literal (strip quotes, canonicalize)
    assignment->literal_node->value = Utilities::StringUtils::removeOuterQuotes(assignment->literal_node->value);
    // find the index of the column
    auto it = std::find(full_col_list.begin(), full_col_list.end(), col_name);
    if (it == full_col_list.end()) {
      return false; // unknown column - validator will report
    }

    std::size_t schema_index = std::distance(full_col_list.begin(), it);
    node.inverse_proj_mask[schema_index] = false; // mark this as a new value
    // map the literal value to the schema index - important for the executor
    node.schema_index_to_literal_map.insert(
        std::pair<std::size_t, std::string>(schema_index, assignment->literal_node->value));
    // 3.2 No duplicate assignments
    if (assigned_columns.find(col_name) != assigned_columns.end()) {
      // duplicate columns, validator will report
      return false;
    }

    // 3.3 Primary key cannot be updated
    if (col_name == primary_key_col) {
      // primary key is not allowed, validator will report
      return false;
    }

    // 3.4 Literal must exist
    if (!assignment->literal_node) {
      // missing literal - validator will report
      return false;
    }
    assigned_columns.insert(assignment->col_name);
  }

  // 4. Normalize WHERE clause (if present)
  if (node.opt_where_node) {
    if (!SemanticNormalizer::normalizeWhereClause(node.opt_where_node, schema)) { return false; }
  }
  node.is_normalized = true;
  return true;
}
