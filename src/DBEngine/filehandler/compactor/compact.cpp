/**
 * Perform the compaction by creating a new table file that is temporarily named table_name_new.bin
 * Then copy all the live records to the new file
 * Then delete the old file
 * Then rename the new file so the name matches the old one.
 */
#include "../public_api.hpp"

void FileHandler::Compactor::compactTable(const std::string& table_name, const ExecutionContext& ctx) {
  FileHandler::ensureTableFileExists(table_name);
  const std::string& old_file_path = FileHandler::getTableFilePath(table_name);
  const std::string new_table_filename = table_name + "-new.bin";
  const std::string new_file_path = FileHandler::getTableFolderPath(table_name) + "/" + table_name + "-new.bin";
  FileHandler::Serializer::createTableFile(new_file_path);

  std::ifstream old_table_file(old_file_path, std::ios::binary);
  std::ofstream new_table_file(new_file_path, std::ios::binary | std::ios::app);

  FileHandler::checkFileValidity(old_table_file, table_name);
  // old file seekg pointer is now at the beginning of the record region
  // at this point, the new file exists and contains the file header. We need to iterate through the old file and copy
  // live records
  const std::size_t number_of_cols_without_pk = ctx.getUntypedTables().at(table_name).size() - 1ul;
  DB_Types::TableFileDeserializationIndicator copy_result;
  do {
    copy_result = FileHandler::Compactor::copyNextLiveRecord(old_table_file, new_table_file, number_of_cols_without_pk);
    if (copy_result == DB_Types::TableFileDeserializationIndicator::IOERROR) {
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::NOCONTX_FILEOPS_COMPACT_UnknownErrorDuringCopyingDuringCompaction);
    }
  } while (copy_result != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE);

  // close the files
  if (new_table_file.is_open()) { new_table_file.close(); }
  if (old_table_file.is_open()) { old_table_file.close(); }
  // delete the old file
  try {
    std::filesystem::remove_all(old_file_path);
  } catch (const std::filesystem::filesystem_error& e) {
    LoggerService::ErrorLogger::handleFatalError(
        StatusCode::FatalErrorCode::COMPACT_FILEOPS_UnknownFileSystemErrorWhileDeletingOldFile,
        std::vector<std::string>{table_name, e.what()});
  }

  // rename the new file to match the old name
  try {
    std::filesystem::rename(new_file_path, old_file_path);
  } catch (std::filesystem::filesystem_error const& error) {
    LoggerService::ErrorLogger::handleFatalError(
        StatusCode::FatalErrorCode::COMPACT_FILEOPS_UnknownFileSystemErrorWhileRenamingNewFile,
        std::vector<std::string>{table_name, error.what()});
  }
}