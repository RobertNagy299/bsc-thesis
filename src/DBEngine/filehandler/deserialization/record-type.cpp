#include "../public_api.hpp"

DB_Types::TableFileDeserializationIndicator
FileHandler::Deserializer::deserializeNextRecordType(std::ifstream& table_file) {
  std::uint64_t record_length;
  // 1. Attempt to read the length indicator
  if (!table_file.read(reinterpret_cast<char*>(&record_length), sizeof(record_length))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return DB_Types::TableFileDeserializationIndicator::ENDOFTABLE;
  }

  // read the record type
  DB_Types::RecordType record_type;
  if (!table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }

  // if it is a tombstone record, return tombstone
  if (record_type == DB_Types::RecordType::DELETE || record_type == DB_Types::RecordType::UNUSED) {
    // adjust the seekg pointer -- we are already 9 bytes into the record, and record length does not contain itself,
    // so we need to jump forward by [record_length - sizeof(record_type)] bytes
    table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
    return DB_Types::TableFileDeserializationIndicator::TOMBSTONE;
  }

  // if it's a live record, skip to the next record and return live
  table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
  return DB_Types::TableFileDeserializationIndicator::LIVE;
}