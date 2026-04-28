#pragma once
#include "./std_logger/message-templates/public-api.hpp"
#include <initializer_list>
#include <iostream>
#include <optional>
#include <string>

namespace LoggerService {
extern bool is_silent_mode;
void setSilentMode(bool silent);

namespace ErrorLogger {
  void handleFatalError(const StatusCode::FatalErrorCode& error_code,
                        DB_Types::status_context_t error_context = std::nullopt);
  void printAsStandardError(const StatusCode::ErrorCode& error_code,
                            DB_Types::status_context_t error_context = std::nullopt);
}; // namespace ErrorLogger

namespace StatusLogger {
  void printTableDescription(const DescribeNode& node, const std::vector<UntypedColumnDefNode*> schema);
  void printAsStandardOutput(const std::string& status_msg);
  void printResultSetAsTable(const SelectNode& node, const std::unique_ptr<DB_Types::ResultSet>& results);
}; // namespace StatusLogger

namespace WarningLogger {
  void printAsStandardOutput(const StatusCode::WarningCode& warning_code,
                             DB_Types::status_context_t warning_context = std::nullopt);
} // namespace WarningLogger

}; // namespace LoggerService