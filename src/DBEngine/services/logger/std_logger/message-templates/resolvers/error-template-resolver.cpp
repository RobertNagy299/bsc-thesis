#include "../public-api.hpp"

std::string MessageTemplateResolver::resolveErrorMessageTemplate(const StatusCode::ErrorCode& error_code,
                                                                 const DB_Types::status_context_t& error_context) {

  std::vector<std::string> resolved_context;
  if (error_context != std::nullopt) { resolved_context = error_context.value(); }
  switch (error_code) {
    case StatusCode::ErrorCode::SEMVAL_TableDoesNotExist: {
      return MessageTemplateResolver::injectContext("Table '{}' does not exist.", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_INSERT_MoreValuesThanColumnsInColList: {
      return MessageTemplateResolver::injectContext(
          "Column list only specifies {} columns, but you tried to insert {} values in a row.", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_INSERT_MoreValuesThanColumnsInTable: {
      return MessageTemplateResolver::injectContext(
          "Table '{}' Only has {} columns, but you tried to insert {} values in a row.", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_INSERT_ColumnDoesNotExistInTable: {
      return MessageTemplateResolver::injectContext("Column '{}' Does not exist in table '{}'", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_INSERT_MoreColumnsInColListThanInTable: {
      return MessageTemplateResolver::injectContext(
          "Table '{}' only has {} columns, but you specified {} columns in the column list of the insert statement.",
          resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_ColumnDoesNotExistInTable: {
      return MessageTemplateResolver::injectContext("Column '{}' does not exist in table '{}'!", resolved_context);
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_PrimaryKeyCannotBeEmpty: {
      return "You've tried to insert an empty primary key value, but the Primary Key cannot be empty.";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_NotNullNoDefaultViolation: {
      return "Cannot insert empty literal into column marked as NOT NULL without explicit DEFAULT value";
    }
    case StatusCode::ErrorCode::SEMVAL_CREATE_TableAlreadyExists: {
      return MessageTemplateResolver::injectContext("Cannot create table '{}', because it already exists.",
                                                    resolved_context);
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_EmptyLiteralIsNotComparable: {
      return "Literal in WHERE condition cannot be empty literal!";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorIsIsNotViolation: {
      return "Comparators 'IS' and 'IS NOT' must be followed by either a 'NULL' or a boolean (TRUE or FALSE) literal!";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorLikeNotLikeViolation: {
      return "Comparators 'LIKE' and 'NOT LIKE' must be followed by a string literal!";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorEQNotEQViolation: {
      return "Comparators '=' and '!=' can only be used with string or numeric literals";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_WHERE_ComparatorMathViolation: {
      return "Mathematical comparators must be followed by a numeric literal!";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_GenericInvalidStatement: {
      return "Insert statement is semantically invalid! The engine will not perform any file operations.";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_SELECT_GenericInvalidStatement: {
      return "Select statement is semantically invalid! The engine will not perform any file operations.";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_UPDATE_GenericInvalidStatement: {
      return "Update statement is semantically invalid! The engine will not perform any file operations.";
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_DELETE_GenericInvalidStatement: {
      return "Delete statement is semantically invalid! The engine will not perform any file operations.";
    }
    case StatusCode::ErrorCode::SEMVAL_INSERT_DuplicatePrimaryKeys: {
      return MessageTemplateResolver::injectContext("Primary key '{}' already exists in table '{}'!", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_UPDATE_TriedUpdatingTheSameColumnInOneCommand: {
      return MessageTemplateResolver::injectContext(
          "Update command contains column '{}' multiple times, which is not allowed.", resolved_context);
    }
    case StatusCode::ErrorCode::SEMVAL_UPDATE_TriedUpdatingThePrimaryKey: {
      return MessageTemplateResolver::injectContext(
          "You've tried to update the primary key (column '{}'), which is not allowed because it's a bad practice.",
          resolved_context);
    }
    case StatusCode::ErrorCode::NOCONTX_SEMVAL_DROP_GenericInvalidStatement: {
      return "Drop statement is semantically invalid! The engine will not perform any file operations.";
    }
    default: {
      return "Unknown error";
    }
  }
}