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

  for (auto& assignment : node.assignment_list_node->assignments) {
    const std::string& col_name = assignment->col_name;

    // 3.1 Column exists
    if (!Utilities::ColumnUtils::columnsExistInTable(schema, col_name)) { return false; }

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
    // 3.5 Normalize literal (strip quotes, canonicalize)
    assignment->literal_node->value = Utilities::StringUtils::removeOuterQuotes(assignment->literal_node->value);
  }

  // 4. Normalize WHERE clause (if present)
  if (node.opt_where_node) {
    if (!SemanticNormalizer::normalizeWhereClause(node.opt_where_node, schema)) { return false; }
  }

  return true;
}
