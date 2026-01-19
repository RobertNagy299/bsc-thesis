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

void ExecutionContext::eraseKeyFromIndex(const std::string& table_name, const std::string& col_name,
                                         const std::string& key_value) {
  if (this && this->indices && this->indices->at(table_name) && this->indices->at(table_name)->at(col_name) &&
      this->indices->at(table_name)->at(col_name)->at(key_value)) {
    this->indices->at(table_name)->at(col_name)->erase(key_value);
  } else {
    LoggerService::ErrorLogger::handleFatalError(
        StatusCode::FatalErrorCode::NOCONTX_HASHIDX_NULLPTR_GenericNullptrError);
  }
}
