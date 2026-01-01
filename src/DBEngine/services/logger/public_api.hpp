#pragma once
#include <iostream>
#include <string>

namespace LoggerService {

namespace ErrorLogger {

  void printAsStandardError(const std::string& error_msg);
}; // namespace ErrorLogger

namespace StatusLogger {

  void printAsStandardOutput(const std::string& status_msg);
}; // namespace StatusLogger

namespace WarningLogger {
  void printAsStandardOutput(const std::string& warning_msg);
} // namespace WarningLogger

}; // namespace LoggerService