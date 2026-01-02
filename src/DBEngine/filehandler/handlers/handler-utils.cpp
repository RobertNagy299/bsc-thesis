#include "../public_api.hpp"

void FileHandler::writeToBinaryFile(std::ofstream& outFile, const std::string& data) {
  // First, write the size of the string so we know how many characters to read later
  std::size_t size = data.size();
  outFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
  // Then, write the actual string data
  outFile.write(data.data(), size);
}

const std::string FileHandler::getTableFolderPath(const std::string& table_name) {
  return FileHandler::DATASTORAGE_BASE_DIRECTORY + "/" + table_name;
}

const std::string FileHandler::getTableFilePath(const std::string& table_name) {
  return FileHandler::getTableFolderPath(table_name) + "/" + table_name + ".bin";
}

void FileHandler::ensureTableFileExists(const std::string& table_name) {
  // ensures parent exists
  std::filesystem::create_directories(FileHandler::DATASTORAGE_BASE_DIRECTORY);
  // Path for this table
  const std::string table_datastorage_dir = FileHandler::getTableFolderPath(table_name);
  std::filesystem::create_directories(table_datastorage_dir);

  const std::string table_datastorage_file_path = FileHandler::getTableFilePath(table_name);
  if (!std::filesystem::exists(std::filesystem::path(table_datastorage_file_path))) {
    std::ofstream table_file(table_datastorage_file_path, std::ios::out | std::ios::binary);
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
                                                     std::vector<std::string>{table_name});
      }

    } catch (const std::exception& exception) {
      // TODO cleanup partially broken file if possible
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::FILEOPS_UnknownExceptionWhileCreatingTableFile,
          std::vector<std::string>{table_name, exception.what()});
    }
  }
}