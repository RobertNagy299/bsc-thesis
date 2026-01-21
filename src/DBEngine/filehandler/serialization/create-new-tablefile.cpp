#include "../public_api.hpp"

/**
 * Creates the table file and populates it with the file header.
 * If the file already exists, it doesn't do anything
 */
void FileHandler::Serializer::createTableFile(const std::string& table_file_path) {
  // create the new file
  if (!std::filesystem::exists(std::filesystem::path(table_file_path))) {
    std::ofstream table_file(table_file_path, std::ios::out | std::ios::binary);
    try {
      if (table_file.is_open()) {
        // insert table file header:
        FileHandler::writeToBinaryFile(table_file, FileHandler::DB_MAGIC);
        FileHandler::writeToBinaryFile(table_file, FileHandler::DB_VERSION);
        FileHandler::writeToBinaryFile(table_file, FileHandler::DB_FLAGS);
        FileHandler::writeToBinaryFile(table_file, FileHandler::DB_RESERVED);
        table_file.close();
      } else {
        LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::FILEOPS_CouldNotCreateTableFile,
                                                     std::vector<std::string>{table_file_path});
      }

    } catch (const std::exception& exception) {
      // TODO cleanup partially broken file if possible
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::FILEOPS_UnknownExceptionWhileCreatingTableFile,
          std::vector<std::string>{table_file_path, exception.what()});
    }
  }
}
