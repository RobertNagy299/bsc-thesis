#pragma once
#include "./std_logger/message-templates/public-api.hpp"
#include <initializer_list>
#include <iostream>
#include <optional>
#include <string>

namespace LoggerService {

namespace ErrorLogger {
  void handleFatalError(const StatusCode::FatalErrorCode& error_code,
                        DB_Types::status_context_t error_context = std::nullopt);
  void printAsStandardError(const StatusCode::ErrorCode& error_code,
                            DB_Types::status_context_t error_context = std::nullopt);
}; // namespace ErrorLogger

namespace StatusLogger {
  void printAsStandardOutput(const std::string& status_msg);
}; // namespace StatusLogger

namespace WarningLogger {
  void printAsStandardOutput(const StatusCode::WarningCode& warning_code,
                             DB_Types::status_context_t warning_context = std::nullopt);
} // namespace WarningLogger

}; // namespace LoggerService