#include "../public_api.hpp"

void ExecutionContext::setUntypedTable(const std::string& table_name,
                                       const std::vector<UntypedColumnDefNode*>& coldefs) {
  this->untyped_tables[table_name] = std::move(coldefs);
}

void ExecutionContext::addNewPrimaryKeyIndex(const std::string& table_name, const std::string& pk_col_name,
                                             const std::string& pk_literal, const std::uint64_t offset) {
  this->indices->at(table_name)->at(pk_col_name)->insert(std::pair<std::string, std::uint64_t>(pk_literal, offset));
}

void ExecutionContext::recalculateIndicesForTable(const std::string& table_name) {
  // first, delete the current indices for the table
  this->indices->erase(table_name);
  // then rebuild them
  this->initializePrimaryKeyIndicesForTable(table_name);
  // TODO add Unique attribute indexing in the future maybe
}