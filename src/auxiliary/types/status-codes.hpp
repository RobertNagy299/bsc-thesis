#pragma once
#include <cstdint>

/**
 * `StatusCodes` are stored as `std::uint16_t` and range from `0 - 65535` (0 to 2^16 - 1)
 * This range is split into 4 parts:
 *
 * - ErrorCodes
 * - FatalErrorCodes
 * - WarningCodes
 * - Reserved region (60000 - 65535) for future use, in case 60 thousand codes are not enough, we can add an extra 5535.
 */
namespace StatusCode {

/**
 * Standard error codes range from `0 to 19999`.
 *
 * Prefix explanation:
 *
 * - `NOCONTX` indicates that this error does not log additional context.
 * - `SEMVAL` means a generic error related to Semantic Validation
 * - `INSERT` means specific error related to insertion logic
 * - `CREATE` means specific error related to creation logic
 * - `WHERE` means specific error related to WHERE clause logic
 */
enum class ErrorCode : std::uint16_t {
  SEMVAL_TableDoesNotExist = 0u,
  SEMVAL_ColumnDoesNotExistInTable,
  SEMVAL_CREATE_TableAlreadyExists,

  NOCONTX_SEMVAL_WHERE_EmptyLiteralIsNotComparable,
  NOCONTX_SEMVAL_WHERE_ComparatorIsIsNotViolation,
  NOCONTX_SEMVAL_WHERE_ComparatorLikeNotLikeViolation,
  NOCONTX_SEMVAL_WHERE_ComparatorEQNotEQViolation,
  NOCONTX_SEMVAL_WHERE_ComparatorMathViolation,

  NOCONTX_SEMVAL_INSERT_GenericInvalidStatement,
  NOCONTX_SEMVAL_SELECT_GenericInvalidStatement,
  NOCONTX_SEMVAL_UPDATE_GenericInvalidStatement,
  NOCONTX_SEMVAL_DELETE_GenericInvalidStatement,

  NOCONTX_SEMVAL_INSERT_PrimaryKeyCannotBeEmpty,
  NOCONTX_SEMVAL_INSERT_NotNullNoDefaultViolation,
  SEMVAL_INSERT_MoreValuesThanColumnsInColList,
  SEMVAL_INSERT_MoreValuesThanColumnsInTable,
  SEMVAL_INSERT_ColumnDoesNotExistInTable,
  SEMVAL_INSERT_MoreColumnsInColListThanInTable,
};

/**
 * Fatal error codes range from 20000 to 39999
 *
 *  Prefix explanation:
 *
 * - `NOCONTX` - indicates that this error does not log additional context.
 * - `FILEOPS` - generic issue related to binary file handling
 * - `NULLPTR` - generic issue related to nullptr dereferencing
 * - `METADAT` - generic issue related to metadata handling
 * - `DROP` - specific issue related to dropping objects
 *
 */
enum class FatalErrorCode : std::uint16_t {
  FILEOPS_GenericFileIOFailure = 20000u,
  FILEOPS_ColOffsetRegionHasMoreColsThanAllowed,
  FILEOPS_CouldNotCreateTableFile,
  FILEOPS_UnknownExceptionWhileCreatingTableFile,
  NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure,
  FILEOPS_UnknownTableFileFormat,
  METADAT_MetadataDirectoryDoesNotExist,
  METADAT_CouldNotCreateMetadataFileForTable,
  METADAT_TableMetadataDirectoryDoesNotExist,
  DROP_UnknownFileSystemError,
};

/**
 * Warning codes range from 40000 to 59999
 *
 * Prefix explanation:
 *
 * - `NOCONTX` - indicates that this warning does not log additional context.
 * - `FILEOPS` - generic issue related to binary file handling
 */
enum class WarningCode : std::uint16_t {
  FILEOPS_FileWasMadeWithDifferentDBVersion = 40000u,
};

} // namespace StatusCode