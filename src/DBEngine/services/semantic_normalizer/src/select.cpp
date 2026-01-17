#include "../public-api.hpp"

bool SemanticNormalizer::normalizeSelect(SelectNode& node, const ExecutionContext& ctx) {
  const auto& tables = ctx.getUntypedTables();
  auto it = tables.find(node.table_name);
  if (it == tables.end()) return false;

  const auto& schema_cols = it->second;
  const std::size_t ncols = schema_cols.size();

  // ---- 1) canonical schema-ordered column list ----
  std::vector<std::string> full_col_list;
  full_col_list.reserve(ncols);
  for (auto* col : schema_cols) full_col_list.push_back(col->name);

  // ---- 2) If user didn't specify → expand '*' ----
  if (!node.columns) { node.columns = new ColumnListNode(full_col_list); }

  // ---- 3) Build projection mask in schema order ----
  node.projection_mask.assign(ncols, false);

  for (const auto& user_col : node.columns->columns) {

    auto it2 = std::find(full_col_list.begin(), full_col_list.end(), user_col);
    if (it2 == full_col_list.end()) {
      return false; // validator will report later
    }

    std::size_t schema_index = std::distance(full_col_list.begin(), it2);
    node.projection_mask[schema_index] = true;
  }

  // ---- 4) Replace column list with canonical schema list ----
  // This means execution always sees full schema, gated by mask
  node.columns->columns = std::move(full_col_list);

  // ---- 5) Normalize WHERE clause ----
  if (!SemanticNormalizer::normalizeWhereClause(node.opt_where_node, schema_cols)) { return false; }

  node.is_normalized = true;
  return true;
}
