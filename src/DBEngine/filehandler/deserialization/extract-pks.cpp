#include "../public_api.hpp"

/* offset is absolute - it does not include the table header - header will always be checked in the local function to
 * ensure format consistency
 * out_offset always points to the start of the record's type, starting from the beginning of
 * the table file
 */
DB_Types::TableFileDeserializationIndicator
FileHandler::Deserializer::deserializeNextPrimaryKey(std::ifstream& table_file, std::string& out_pk_val,
                                                     std::uint64_t& out_offset,
                                                     const std::size_t number_of_columns_without_pk) {
  std::uint64_t record_length;
  // 1. Attempt to read the length indicator
  if (!table_file.read(reinterpret_cast<char*>(&record_length), sizeof(record_length))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return DB_Types::TableFileDeserializationIndicator::ENDOFTABLE;
  }
  // at the start of the scanning of the record, reposition the offset
  // tellg will include the 32 bytes long file header and the 8 bytes long record length.
  // our binary file format ensures that the column offset region starts right after the record_type byte.

  out_offset = static_cast<std::uint64_t>(table_file.tellg());

  // read the record type
  DB_Types::RecordType record_type;
  if (!table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }

  // if it is a tombstone record, skip it
  if (record_type == DB_Types::RecordType::DELETE || record_type == DB_Types::RecordType::UNUSED) {
    // adjust the seekg pointer -- we are already 9 bytes into the record, and record length does not contain itself,
    // so we need to jump forward by [record_length - sizeof(record_type)] bytes
    table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
    return DB_Types::TableFileDeserializationIndicator::TOMBSTONE;
  }
  std::uint64_t col_offset_region_length = number_of_columns_without_pk * sizeof(DB_Types::column_offset_t);
  // at this point, this is a valid insert / update record. Let's skip through the col offset region
  table_file.seekg(col_offset_region_length, std::ios::cur);
  // get the primary key
  std::uint64_t pk_len;
  if (!table_file.read(reinterpret_cast<char*>(&pk_len), sizeof(pk_len))) {
    // TODO handle error, exit(1)
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }
  // 2. Resize the string to hold the upcoming data
  out_pk_val.resize(pk_len);
  // 3. Read the actual data bytes into the resized string's buffer
  // &record[0] is guaranteed to be a pointer to the start of the string's internal buffer since C++11
  if (!table_file.read(&out_pk_val[0], pk_len)) {
    // If data read fails, handle the error (optional: resize back or clear)
    out_pk_val.clear();
    // TODO error handling
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }
  // move seekg to the start of the next record
  // we are currently at the beginning of the data tuple region
  std::uint64_t remaining_bytes =
      record_length - (sizeof(DB_Types::RecordType) + col_offset_region_length + sizeof(pk_len) + out_pk_val.size());
  table_file.seekg(remaining_bytes, std::ios::cur);
  return DB_Types::TableFileDeserializationIndicator::LIVE;
}
// table header was already read by this point, so the seekg pointer is 32 bytes ahead of file.beg
/**
 * @param number_of_columns_without_pk Should already be adjusted to exclude the primary key -
 * it is used to adjust the seekg pointer to skip the column offset region
 * @param table_file read only binary stream
 */
DB_Types::index_ptr_t FileHandler::extractPrimaryKeysIndex(std::ifstream& table_file,
                                                           const std::size_t number_of_columns_without_pk) {
  auto result = std::make_unique<DB_Types::index_t>();
  std::string pk_val;
  std::uint64_t offset = 0ul;
  auto serialization_indicator_result =
      FileHandler::Deserializer::deserializeNextPrimaryKey(table_file, pk_val, offset, number_of_columns_without_pk);
  // iterate through every record in the file and read the primary key
  while (serialization_indicator_result != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE) {
    if (serialization_indicator_result == DB_Types::TableFileDeserializationIndicator::LIVE) {
      result->insert(std::pair<std::string, std::uint64_t>(pk_val, offset));
    }
    serialization_indicator_result =
        FileHandler::Deserializer::deserializeNextPrimaryKey(table_file, pk_val, offset, number_of_columns_without_pk);
  }

  return std::move(result);
}