#include "../public_api.hpp"

void ExecutionContext::setUntypedTable(const std::string& table_name,
                                       const std::vector<UntypedColumnDefNode*>& coldefs) {
  this->untyped_tables[table_name] = std::move(coldefs);
}

void ExecutionContext::upsertPrimaryKeyIndex(const std::string& table_name, const std::string& pk_col_name,
                                             const std::string& pk_literal, const std::uint64_t offset) {
  const auto currentOffset = this->indices->at(table_name)->at(pk_col_name)->find(pk_literal);
  if (currentOffset != this->indices->at(table_name)->at(pk_col_name)->end()) {
    this->indices->at(table_name)->at(pk_col_name)->at(pk_literal) = offset;
  } else {
    this->indices->at(table_name)->at(pk_col_name)->insert(std::pair<std::string, std::uint64_t>(pk_literal, offset));
  }
}

void ExecutionContext::recalculateIndicesForTable(const std::string& table_name) {
  // first, delete the current indices for the table
  if (this->indices->find(table_name) != this->indices->end()) { this->indices->erase(table_name); }
  // then rebuild them
  this->initializePrimaryKeyIndicesForTable(table_name);
  // TODO add Unique attribute indexing in the future maybe
}