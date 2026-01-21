/**
 * Calculate the ratio of tombstone / live records
 * If the ratio is above a treshold, perform compaction.
 */
#include "../public_api.hpp"

double FileHandler::Compactor::calculateTombstoneRatio(const std::string& table_name) {
  FileHandler::ensureTableFileExists(table_name);
  const std::string& file_path = FileHandler::getTableFilePath(table_name);
  std::ifstream table_file(file_path, std::ios::binary);

  FileHandler::checkFileValidity(table_file, table_name);
  // seekg pointer is now at the beginning of the record region
  std::uint64_t tombstone_count = 0, total_count = 0;
  if (table_file.is_open()) {
    DB_Types::TableFileDeserializationIndicator type_deser_result;
    do {
      type_deser_result = FileHandler::Deserializer::deserializeNextRecordType(table_file);
      total_count++;
      if (type_deser_result == DB_Types::TableFileDeserializationIndicator::TOMBSTONE) { tombstone_count++; }
      if (type_deser_result == DB_Types::TableFileDeserializationIndicator::IOERROR) {
        LoggerService::ErrorLogger::handleFatalError(
            StatusCode::FatalErrorCode::NOCONTX_FILEOPS_UnknownIoErrorDuringDeserialization);
      }
    } while (type_deser_result != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE);
  }

  if (table_file.is_open()) { table_file.close(); }

  return 1.0 * tombstone_count / total_count;
}
