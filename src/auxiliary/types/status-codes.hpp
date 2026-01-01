#pragma once
#include <cstdint>

namespace StatusCode {

/**
 * @brief standard errors range from 0-19999.
 *
 *  prefix explanation:
 *
 * - NOCONTX - indicates that this error does not log additional context.
 *
 * - SEMVAL means a generic error related to Semantic Validation
 *
 * - INSERT means specific error related to insertion logic
 *  */
enum class ErrorCode {
  SEMVAL_TableDoesNotExist = 0u,
  INSERT_MoreValuesThanColumnsInColList,
  INSERT_MoreValuesThanColumnsInTable,
  INSERT_ColumnDoesNotExistInTable,
  INSERT_MoreColumnsInColListThanInTable,
};

/**
 * @brief fatal error codes range from 20000 - 39999
 *
 * prefix explanation:
 *
 * - NOCONTX - indicates that this error does not log additional context.
 *
 * - FILEOPS - generic issue related to binary file handling
 *
 * - NULLPTR - generic issue related to nullptr dereferencing
 */
enum class FatalErrorCode {
  FILEOPS_GenericFileIOFailure = 20000u,
  FILEOPS_ColOffsetRegionHasMoreColsThanAllowed,
  NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure,
  FILEOPS_UnknownTableFileFormat,
};

enum class WarningCode : std::uint16_t {

};

} // namespace StatusCode