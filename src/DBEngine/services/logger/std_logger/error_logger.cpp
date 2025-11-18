#include "../public_api.hpp"

void LoggerService::ErrorLogger::printAsStandardError(const std::string& error_msg) {
  std::cerr << "\033[31m" << error_msg << "\033[0m" << std::endl;
}
