#include "../public-api.hpp"

std::string MessageTemplateResolver::resolveWarningMessageTemplate(const StatusCode::WarningCode& warning_code,
                                                                   const DB_Types::status_context_t& warning_context) {
  std::vector<std::string> resolved_context;
  if (warning_context != std::nullopt) { resolved_context = warning_context.value(); }
  switch (warning_code) {
    case StatusCode::WarningCode::FILEOPS_FileWasMadeWithDifferentDBVersion: {
      return MessageTemplateResolver::injectContext("Table file for table '{}' was created with a different version of "
                                                    "the DB engine. Expect undefined behavior. Use at your own risk!",
                                                    resolved_context);
    }
    case StatusCode::WarningCode::CSV_IMPORT_RowHasMoreColumnsThanTable: {
      return MessageTemplateResolver::injectContext(
          "Found a row with '{}' columns on line '{}' of the csv file. This is more than the number of columns in "
          "table '{}' (which is "
          "'{}'). The "
          "system is skipping this row, and will continue trying to import other rows.",
          resolved_context);
    }
    default: {
      return "Unknown warning";
    }
  }
}