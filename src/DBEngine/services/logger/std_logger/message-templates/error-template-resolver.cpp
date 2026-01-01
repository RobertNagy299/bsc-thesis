#include "./public-api.hpp"

std::string MessageTemplateResolver::resolveErrorMessageTemplate(const StatusCode::ErrorCode& error_code,
                                                                 DB_Types::status_context_t error_context) {

  std::vector<std::string> resolved_context;
  if (error_context != std::nullopt) { resolved_context = error_context.value(); }
  switch (error_code) {
    case StatusCode::ErrorCode::SEMVAL_TableDoesNotExist: {
      return MessageTemplateResolver::injectContext("Table {} does not exist.", resolved_context);
    }
    case StatusCode::ErrorCode::INSERT_MoreValuesThanColumnsInColList: {
      return MessageTemplateResolver::injectContext(
          "Column list only specifies {} columns, but you tried to insert {} values in a row.", resolved_context);
    }
    case StatusCode::ErrorCode::INSERT_MoreValuesThanColumnsInTable: {
      return MessageTemplateResolver::injectContext(
          "{} Only has {} columns, but you tried to insert {} values in a row.", resolved_context);
    }
    case StatusCode::ErrorCode::INSERT_ColumnDoesNotExistInTable: {
      return MessageTemplateResolver::injectContext("Column {} Does not exist in table {}", resolved_context);
    }
    case StatusCode::ErrorCode::INSERT_MoreColumnsInColListThanInTable: {
      return MessageTemplateResolver::injectContext("Table {} has {} columns, but you tried to insert {} columns.",
                                                    resolved_context);
    }
  }
}