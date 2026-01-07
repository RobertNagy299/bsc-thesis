#include "../public_api.hpp"

const DB_Types::untyped_table_t& ExecutionContext::getUntypedTables() const { return this->untyped_tables; }
const DB_Types::table_colcode_map_t& ExecutionContext::getTableColcodeMap() const { return this->table_colcodes; }

ExecutionContext& ExecutionContext::getInstance() {
  if (!instance) { instance = new ExecutionContext(); }
  return *instance;
}

DB_Types::untyped_table_t& ExecutionContext::transferOwnershipOfUntypedTables() { return this->untyped_tables; }

const DB_Types::indices_ptr_t& ExecutionContext::getHashmapIndices() const { return this->indices; }