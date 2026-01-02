#include "../public_api.hpp"

void ExecutionContext::eraseTable(const DB_Types::untyped_table_t::iterator& it) { this->untyped_tables.erase(it); }
void ExecutionContext::eraseTable(const std::string& table_name) { this->untyped_tables.erase(table_name); }

void ExecutionContext::eraseInMemoryHashMapIndicesForTable(const std::string& table_name) {
  if (this != nullptr && this->indices != nullptr) {
    this->indices->erase(table_name);
  } else {
    LoggerService::ErrorLogger::handleFatalError(
        StatusCode::FatalErrorCode::NOCONTX_HASHIDX_NULLPTR_GenericNullptrError);
  }
}