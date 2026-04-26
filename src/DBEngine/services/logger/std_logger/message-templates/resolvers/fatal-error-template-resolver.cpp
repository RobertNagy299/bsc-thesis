#include "../public-api.hpp"

std::string MessageTemplateResolver::resolveFatalErrorMessageTemplate(const StatusCode::FatalErrorCode& error_code,
                                                                      const DB_Types::status_context_t& error_context) {

  std::vector<std::string> resolved_context;
  if (error_context != std::nullopt) { resolved_context = error_context.value(); }
  switch (error_code) {
    case StatusCode::FatalErrorCode::FILEOPS_GenericFileIOFailure: {
      return MessageTemplateResolver::injectContext("An unknown IO error occured. Error message: {}", resolved_context);
    }
    case StatusCode::FatalErrorCode::NOCONTX_CSV_IMPORT_CannotOpenFile: {
      return "Could not open the given CSV file. Check your file path.";
    }
    case StatusCode::FatalErrorCode::FILEOPS_ColOffsetRegionHasMoreColsThanAllowed: {
      return MessageTemplateResolver::injectContext(
          "Table '{}' Has more than 256 columns on the disk, which is illegal.", resolved_context);
    }
    case StatusCode::FatalErrorCode::CSV_IMPORT_UnknownException: {
      return MessageTemplateResolver::injectContext("Unknown exception while parsing the CSV file. Error message: '{}'",
                                                    resolved_context);
    }
    case StatusCode::FatalErrorCode::NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure: {
      return "One of the pointers responsible for primary key indexing was nullptr.";
    }
    case StatusCode::FatalErrorCode::FILEOPS_UnknownTableFileFormat: {
      return MessageTemplateResolver::injectContext("Unknown table file format for table '{}'.", resolved_context);
    }
    case StatusCode::FatalErrorCode::METADAT_MetadataDirectoryDoesNotExist: {
      return MessageTemplateResolver::injectContext(
          "Metadata directory does not exist. The supposed location was: '{}' ", resolved_context);
    }
    case StatusCode::FatalErrorCode::METADAT_CouldNotCreateMetadataFileForTable: {
      return MessageTemplateResolver::injectContext("Could not create metadata file for table '{}'!", resolved_context);
    }
    case StatusCode::FatalErrorCode::METADAT_TableMetadataDirectoryDoesNotExist: {
      return MessageTemplateResolver::injectContext("Metadata directory for table '{}' does not exist.",
                                                    resolved_context);
    }
    case StatusCode::FatalErrorCode::DROP_FILEOPS_UnknownFileSystemError: {
      return MessageTemplateResolver::injectContext(
          "Unknown filesystem error while attempting to drop table '{}'. Error message: '{}'", resolved_context);
    }
    case StatusCode::FatalErrorCode::COMPACT_FILEOPS_UnknownFileSystemErrorWhileDeletingOldFile: {
      return MessageTemplateResolver::injectContext("Unknown filesystem error while attempting to compact table '{}'. "
                                                    "Occured while trying to delete the old file. Error message: '{}'",
                                                    resolved_context);
    }
    case StatusCode::FatalErrorCode::COMPACT_FILEOPS_UnknownFileSystemErrorWhileRenamingNewFile: {
      return MessageTemplateResolver::injectContext("Unknown filesystem error while attempting to compact table '{}'. "
                                                    "Occured while trying to rename the new file. Error message: '{}'",
                                                    resolved_context);
    }
    case StatusCode::FatalErrorCode::FILEOPS_CouldNotCreateTableFile: {
      return MessageTemplateResolver::injectContext("Could not create binary file for table '{}'!", resolved_context);
    }
    case StatusCode::FatalErrorCode::FILEOPS_UnknownExceptionWhileCreatingTableFile: {
      return MessageTemplateResolver::injectContext(
          "Unknown exception while creating binary file for table '{}'. Error message: '{}'", resolved_context);
    }
    case StatusCode::FatalErrorCode::NOCONTX_FILEOPS_COMPACT_UnknownErrorDuringCopyingDuringCompaction: {
      return "Unknown IOERROR while copying live records to new file";
    }
    case StatusCode::FatalErrorCode::NOCONTX_HASHIDX_NULLPTR_GenericNullptrError: {
      return "Could not operate on the in-memory hash-map index due to nullptr error";
    }
    case StatusCode::FatalErrorCode::NOCONTX_FILEOPS_UnknownIoErrorDuringDeserialization: {
      return "Unknown IOError during binary file handling";
    }
    case StatusCode::FatalErrorCode::NOCONTX_FILEOPS_DELETE_ColumnNotFoundDueToCorruption: {
      return "Could not deserialize column offset region during delete process, possibly due to file corruption or "
             "misalignment";
    }
    case StatusCode::FatalErrorCode::COLCODE_TableDoesNotExistWhenTryingToConstructColcodeMapping: {
      return MessageTemplateResolver::injectContext(
          "Could not find table '{}' while trying to calculate its column codes.", resolved_context);
    }
    default: {
      return "Unknown fatal error";
    }
  }
}
