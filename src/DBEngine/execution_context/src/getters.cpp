#include "../public_api.hpp"

const DB_Types::untyped_table_t& ExecutionContext::getUntypedTables() const { return this->untyped_tables; }
const DB_Types::table_colcode_map_t& ExecutionContext::getTableColcodeMap() const { return this->table_colcodes; }

ExecutionContext& ExecutionContext::getInstance() {
  if (!instance) { instance = new ExecutionContext(); }
  return *instance;
}

DB_Types::untyped_table_t& ExecutionContext::transferOwnershipOfUntypedTables() { return this->untyped_tables; }

const DB_Types::indices_ptr_t& ExecutionContext::getHashmapIndices() const { return this->indices; }

std::uint64_t ExecutionContext::getDeleteCountForTable(const std::string& table_name) {
  auto it = this->delete_counter_per_table.find(table_name);
  if (it == delete_counter_per_table.end()) {
    LoggerService::ErrorLogger::handleFatalError(
        StatusCode::FatalErrorCode::COMPACT_INTERNAL_TableNotFoundInDeleteCountMap,
        std::vector<std::string>{table_name});
  }
  return it->second;
}
