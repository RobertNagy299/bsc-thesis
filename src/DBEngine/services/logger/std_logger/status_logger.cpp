#include "../public_api.hpp"

void LoggerService::StatusLogger::printAsStandardOutput(const std::string& status_msg) {
  std::cout << status_msg << std::endl;
}