#include "../public_api.hpp"

/**
 * @brief Used for handling user errors that are expected (such as specifying a table that doesn't exist, or specifying
 * column names that don't exist in a table, etc.). It prints the error to std::cerr
 */
void LoggerService::ErrorLogger::printAsStandardError(const StatusCode::ErrorCode& error_code,
                                                      DB_Types::status_context_t error_context) {
  std::cerr << "\033[31m" << "ERROR: (Code : " << std::to_string(static_cast<std::uint16_t>(error_code)) << "-E) "
            << MessageTemplateResolver::resolveErrorMessageTemplate(error_code, error_context) << "\033[0m"
            << std::endl;
}

/**
 * @brief Used for handling unexpected critical issues, such as file corruptions, unexpected null pointers, incorrect
 * file formatting, etc. It prints the error to std::cerr and calls std::exit(1).
 *
 * This method should be the only place in the system that calls std::exit(1), terminating the program.
 */
void LoggerService::ErrorLogger::handleFatalError(const StatusCode::FatalErrorCode& error_code,
                                                  DB_Types::status_context_t error_context) {
  std::cerr << "\033[31m" << "FATAL ERROR: (Code : " << std::to_string(static_cast<std::uint16_t>(error_code)) << "-F) "
            << MessageTemplateResolver::resolveFatalErrorMessageTemplate(error_code, error_context)
            << "\nTerminating..." << "\033[0m" << std::endl;
  std::exit(1);
}
