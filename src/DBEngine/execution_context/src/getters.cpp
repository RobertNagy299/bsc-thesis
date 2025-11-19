#include "../public_api.hpp"

const untyped_table_t& ExecutionContext::getUntypedTables() const { return this->untyped_tables; }

ExecutionContext& ExecutionContext::getInstance() {
  if (!instance) { instance = new ExecutionContext(); }
  return *instance;
}