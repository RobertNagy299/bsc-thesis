#include "../public_api.hpp"

void LoggerService::WarningLogger::printAsStandardOutput(const std::string& warning_message) {
  std::cout << "\033[33m" << warning_message << "\033[0m" << std::endl;
}