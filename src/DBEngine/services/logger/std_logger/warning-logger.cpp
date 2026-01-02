#include "../public_api.hpp"

void LoggerService::WarningLogger::printAsStandardOutput(const StatusCode::WarningCode& warning_code,
                                                         DB_Types::status_context_t warning_context) {
  std::cout << "\033[33m" << "Warning: (Code : " << std::to_string(static_cast<std::uint16_t>(warning_code)) << "-W) "
            << MessageTemplateResolver::resolveWarningMessageTemplate(warning_code, warning_context) << "\033[0m"
            << std::endl;
}