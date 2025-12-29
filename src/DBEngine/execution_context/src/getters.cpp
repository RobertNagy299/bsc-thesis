#include "../public_api.hpp"

const untyped_table_t& ExecutionContext::getUntypedTables() const { return this->untyped_tables; }
const table_colcode_map_t& ExecutionContext::getTableColcodeMap() const { return this->table_colcodes; }

ExecutionContext& ExecutionContext::getInstance() {
  if (!instance) { instance = new ExecutionContext(); }
  return *instance;
}