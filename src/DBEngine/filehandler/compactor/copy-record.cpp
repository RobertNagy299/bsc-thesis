#include "../public_api.hpp"

/**
 * This function is responsible for copying a live record from the old file to the new one
 */
DB_Types::TableFileDeserializationIndicator
FileHandler::Compactor::copyNextLiveRecord(std::ifstream& old_table_file, std::ofstream& new_table_file,
                                           const std::size_t& number_of_cols_without_pk) {
  std::uint64_t record_length;
  // 1. Attempt to read the length indicator
  if (!old_table_file.read(reinterpret_cast<char*>(&record_length), sizeof(record_length))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return DB_Types::TableFileDeserializationIndicator::ENDOFTABLE;
  }

  // read the record type
  DB_Types::RecordType record_type;
  if (!old_table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }

  // if it is a tombstone record, skip it
  if (record_type == DB_Types::RecordType::DELETE || record_type == DB_Types::RecordType::UNUSED) {
    // adjust the seekg pointer -- we are already 9 bytes into the record, and record length does not contain itself,
    // so we need to jump forward by [record_length - sizeof(record_type)] bytes
    old_table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
    return DB_Types::TableFileDeserializationIndicator::TOMBSTONE;
  }
  // we know now that it is a live record, let's read everything and copy it to the new file
  std::vector<DB_Types::column_offset_t> column_offsets;
  column_offsets.reserve(number_of_cols_without_pk);
  for (std::size_t i = 0; i < number_of_cols_without_pk; ++i) {
    DB_Types::column_offset_t col_offset;
    old_table_file.read(reinterpret_cast<char*>(&col_offset), sizeof(col_offset));
    column_offsets.push_back(col_offset);
  }
  // read the pk
  std::uint64_t pk_size;
  std::string pk_literal;
  old_table_file.read(reinterpret_cast<char*>(&pk_size), sizeof(pk_size));
  pk_literal.resize(pk_size);
  old_table_file.read(&pk_literal[0], pk_size);
  // read the rest of the literals
  std::vector<std::string> literals;
  literals.reserve(number_of_cols_without_pk);
  for (std::size_t i = 0; i < number_of_cols_without_pk; ++i) {
    std::uint64_t literal_size;
    std::string literal;
    old_table_file.read(reinterpret_cast<char*>(&literal_size), sizeof(literal_size));
    literal.resize(literal_size);
    old_table_file.read(&literal[0], literal_size);
    literals.push_back(literal);
  }
  // we've read the whole record. Let's paste it into the old one
  try {
    FileHandler::writeToBinaryFile(new_table_file, record_length);
    FileHandler::writeToBinaryFile(new_table_file, record_type);

    for (auto& off : column_offsets) FileHandler::writeToBinaryFile(new_table_file, off);

    FileHandler::writeToBinaryFile(new_table_file, pk_literal);

    for (auto& lit : literals) FileHandler::writeToBinaryFile(new_table_file, lit);
  } catch (const std::exception& exc) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::FILEOPS_GenericFileIOFailure,
                                                 std::vector<std::string>{exc.what()});
  }

  return DB_Types::TableFileDeserializationIndicator::LIVE;
}