#include "../public_api.hpp"

const bool SemanticValidator::checkIfTableExists(const std::string& table_name, const ExecutionContext& ctx) {
  if (ctx.getUntypedTables().find(table_name) == ctx.getUntypedTables().end()) {
    LoggerService::ErrorLogger::printAsStandardError(StatusCode::ErrorCode::SEMVAL_TableDoesNotExist,
                                                     std::vector<std::string>{table_name});
    return false;
  }
  return true;
}
