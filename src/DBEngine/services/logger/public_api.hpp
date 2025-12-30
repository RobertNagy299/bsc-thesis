#pragma once
#include <iostream>
#include <string>
// Static method and module container
namespace LoggerService {

// LoggerService() = delete;
/// LoggerService(const LoggerService&) = delete;
// LoggerService& operator=(const LoggerService&) = delete;

namespace ErrorLogger {
// ErrorLogger() = delete;
// ErrorLogger(const ErrorLogger&) = delete;
// ErrorLogger& operator=(const ErrorLogger&) = delete;

void printAsStandardError(const std::string& error_msg);
}; // namespace ErrorLogger

namespace StatusLogger {
// StatusLogger() = delete;
// StatusLogger(const StatusLogger&) = delete;
// StatusLogger& operator=(const StatusLogger&) = delete;

void printAsStandardOutput(const std::string& status_msg);
}; // namespace StatusLogger

namespace WarningLogger {
void printAsStandardOutput(const std::string& warning_msg);
}
}; // namespace LoggerService