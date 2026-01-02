#pragma once
#include "../../../../../auxiliary/types/status-codes.hpp"
#include "../../../../../auxiliary/types/types.hpp"
#include <string>

namespace MessageTemplateResolver {
std::string injectContext(std::string&& msg_template, const std::vector<std::string> resolved_context);
std::string resolveErrorMessageTemplate(const StatusCode::ErrorCode& error_code,
                                        const DB_Types::status_context_t& error_context = std::nullopt);
std::string resolveFatalErrorMessageTemplate(const StatusCode::FatalErrorCode& error_code,
                                             const DB_Types::status_context_t& error_context = std::nullopt);
std::string resolveWarningMessageTemplate(const StatusCode::WarningCode& warning_code,
                                          const DB_Types::status_context_t& warning_context = std::nullopt);
} // namespace MessageTemplateResolver