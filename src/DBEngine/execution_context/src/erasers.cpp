#include "../public_api.hpp"

void ExecutionContext::eraseTable(const DB_Types::untyped_table_t::iterator& it) { this->untyped_tables.erase(it); }
