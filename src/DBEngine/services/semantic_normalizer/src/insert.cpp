#include "../public-api.hpp"

/**
 * Modifies the InsertNode argument
 *
 */
bool SemanticNormalizer::normalizeInsert(InsertNode& node, const ExecutionContext& ctx) {
  const auto& tables = ctx.getUntypedTables();
  auto t = tables.find(node.table_name);
  if (t == tables.end()) {
    // TODO move this to the validator instead and decouple their calling in the visitor
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{node.table_name});
    return false;
  } // validator will error later

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

    std::vector<LiteralNode*> normalized_row(ncols, nullptr);
    std::cout << "In semantic normalizer, node.cols size = " << std::to_string(node.columns->columns.size()) << '\n';
    for (std::size_t j = 0; j < node.columns->columns.size(); ++j) {
      std::size_t schema_index = std::distance(
          full_col_list.begin(), std::find(full_col_list.begin(), full_col_list.end(), node.columns->columns[j]));

      if (j < record->values.size())
        normalized_row[schema_index] = record->values[j];
      else
        normalized_row[schema_index] = new LiteralNode(LiteralNode::Type::EMPTY, ""); // implicit default
    }
    std::cout << "size of normalized_row = " << std::to_string(normalized_row.size()) << '\n';
    record->values = std::move(normalized_row);
  }

  // ---- 4) Replace user column list with canonical schema list ----
  node.columns->columns = std::move(full_col_list);

  node.is_normalized = true;
  return true;
}
