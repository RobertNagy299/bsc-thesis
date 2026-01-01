#include "../public_api.hpp"

void LoggerService::ErrorLogger::printAsStandardError(const StatusCode::ErrorCode& error_code,
                                                      DB_Types::status_context_t error_context) {
  std::cerr << "\033[31m" << "ERROR: (Code : " << std::to_string(static_cast<std::uint16_t>(error_code)) << "-E) "
            << MessageTemplateResolver::resolveErrorMessageTemplate(error_code, error_context) << "\033[0m"
            << std::endl;
}

/**
 * @brief calls std::exit(1), terminating the program. It also logs the error to std::cerr.
 */
void LoggerService::ErrorLogger::handleFatalError(const StatusCode::FatalErrorCode& error_code,
                                                  DB_Types::status_context_t error_context) {
  std::cerr << "\033[31m" << "FATAL ERROR: (Code : " << std::to_string(static_cast<std::uint16_t>(error_code)) << "-F) "
            << MessageTemplateResolver::resolveFatalErrorMessageTemplate(error_code, error_context)
            << "\nTerminating..." << "\033[0m" << std::endl;
  std::exit(1);
}
