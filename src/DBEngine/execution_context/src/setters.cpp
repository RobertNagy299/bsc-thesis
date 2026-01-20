#include "../public_api.hpp"

void ExecutionContext::setUntypedTable(const std::string& table_name,
                                       const std::vector<UntypedColumnDefNode*>& coldefs) {
  this->untyped_tables[table_name] = std::move(coldefs);
}

void ExecutionContext::recalculateIndicesForTable(const std::string& table_name) {
  // first, delete the current indices for the table
  this->indices->erase(table_name);
  // then rebuild them
  this->initializePrimaryKeyIndicesForTable(table_name);
  // TODO add Unique attribute indexing in the future maybe
}