#include "../public_api.hpp"

void FileHandler::writeToBinaryFile(std::ofstream& outFile, const std::string& data) {
  // First, write the size of the string so we know how many characters to read later
  size_t size = data.size();
  outFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
  // Then, write the actual string data
  outFile.write(data.data(), size);
}

template <typename T> void FileHandler::writeToBinaryFile(std::ofstream& outFile, const T& data) {
  outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

void FileHandler::ensureTableFileExists(const std::string& table_name) {
  // ensures parent exists
  std::filesystem::create_directories(FileHandler::DATASTORAGE_BASE_DIRECTORY);
  // Path for this table
  const std::string table_datastorage_dir = FileHandler::DATASTORAGE_BASE_DIRECTORY + "/" + table_name;
  std::filesystem::create_directories(table_datastorage_dir);

  const std::string table_datastorage_file = table_datastorage_dir + "/" + table_name + ".bin";
  if (!std::filesystem::exists(std::filesystem::path(table_datastorage_file))) {
    std::ofstream table_file(table_datastorage_file, std::ios::out | std::ios::binary);
    if (table_file.is_open()) {
      // insert table file header:
      FileHandler::writeToBinaryFile(table_file, FileHandler::DB_MAGIC);
      FileHandler::writeToBinaryFile(table_file, FileHandler::DB_VERSION);
      FileHandler::writeToBinaryFile(table_file, FileHandler::DB_FLAGS);
      FileHandler::writeToBinaryFile(table_file, FileHandler::DB_RESERVED);
      table_file.close();
    } else {
      LoggerService::ErrorLogger::printAsStandardError("ERROR: Could not create file: " + table_datastorage_file);
    }
  }
}