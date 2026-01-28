#include "../public-api.hpp"

/**
 * Modifies the InsertNode argument
 *
 */
bool SemanticNormalizer::normalizeInsert(InsertNode& node, const ExecutionContext& ctx) {
  const auto& tables = ctx.getUntypedTables();
  auto t = tables.find(node.table_name);
  if (t == tables.end()) { return false; } // validator will report the error later

  const auto& schema_cols = t->second;
  const std::size_t ncols = schema_cols.size();

  // ---- 1) Build canonical column list in schema order ----
  std::vector<std::string> full_col_list;
  full_col_list.reserve(ncols);
  for (auto& col : schema_cols) full_col_list.push_back(col->name);

  // If user didn't specify columns → fill in all of them
  if (!node.columns) { node.columns = new ColumnListNode(full_col_list); }

  // ---- 2) Build projection mask ----
  node.projection_mask.assign(ncols, false);

  for (std::size_t i = 0; i < node.columns->columns.size(); ++i) {
    const auto& name = node.columns->columns[i];

    // find index in schema
    auto it = std::find(full_col_list.begin(), full_col_list.end(), name);
    if (it == full_col_list.end()) {
      LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_ColumnDoesNotExistInTable,
                                                       std::vector<std::string>{name, node.table_name});
      return false; // unknown colum
    }

    std::size_t schema_index = std::distance(full_col_list.begin(), it);
    node.projection_mask[schema_index] = true;
  }

  // ---- 3) Rewrite each VALUES(...) record to schema order ----
  for (auto& record : node.values->records) {
    // check if the user gave more values than there are columns in the table
    if (record->values.size() > ncols) { return false; }
    // construct the normalized value records
    std::vector<LiteralNode*> normalized_row(ncols, nullptr);
    for (std::size_t j = 0; j < node.columns->columns.size(); ++j) {
      std::size_t schema_index = std::distance(
          full_col_list.begin(), std::find(full_col_list.begin(), full_col_list.end(), node.columns->columns[j]));

      if (j < record->values.size())
        normalized_row[schema_index] = record->values[j];
      else
        normalized_row[schema_index] = new LiteralNode(LiteralNode::Type::EMPTY, ""); // implicit default
    }
    record->values = std::move(normalized_row);
  }

  // ---- 4) Replace user column list with canonical schema list ----
  node.columns->columns = std::move(full_col_list);

  node.is_normalized = true;
  return true;
}
