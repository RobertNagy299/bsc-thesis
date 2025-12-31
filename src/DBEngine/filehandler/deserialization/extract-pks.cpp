#include "../public_api.hpp"

// offset is absolute - it does not include the table header - header will always be checked in the local function to
// ensure format consistency
TableFileDeserializationIndicator FileHandler::deserializeNextPrimaryKey(std::ifstream& table_file,
                                                                         std::string& out_pk_val,
                                                                         std::uint64_t& out_offset,
                                                                         std::uint64_t& out_next_offset_start,
                                                                         const size_t number_of_columns_without_pk) {
  std::size_t record_length;
  // 1. Attempt to read the length indicator
  if (!table_file.read(reinterpret_cast<char*>(&record_length), sizeof(std::size_t))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return TableFileDeserializationIndicator::ENDOFTABLE;
  }

  // read the record type
  RecordType record_type;
  if (!table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return TableFileDeserializationIndicator::IOERROR;
  }
  size_t offset_delta = sizeof(record_length) + sizeof(RecordType);

  // if it is a tombstone record, skip it
  if (record_type == RecordType::DELETE || record_type == RecordType::UNUSED) {
    // adjust the seekg pointer -- we are already 9 bytes into the record, and record length does not contain itself,
    // so we need to jump forward by [record_length - 1] bytes
    table_file.seekg(record_length - 1UL, std::ios::cur);
    // adjust the offsets
    out_offset += offset_delta;
    out_next_offset_start = record_length - offset_delta;
    return TableFileDeserializationIndicator::TOMBSTONE;
  }
  // at this point, this is a valid insert / update record. Let's skip through the col offset region
  table_file.seekg(number_of_columns_without_pk * sizeof(column_offset_t), std::ios::cur);
  // get the primary key
  size_t pk_len;
  if (!table_file.read(reinterpret_cast<char*>(&pk_len), sizeof(pk_len))) {
    return TableFileDeserializationIndicator::IOERROR;
  }
  // 2. Resize the string to hold the upcoming data
  out_pk_val.resize(pk_len);
  // 3. Read the actual data bytes into the resized string's buffer
  // &record[0] is guaranteed to be a pointer to the start of the string's internal buffer since C++11
  if (!table_file.read(&out_pk_val[0], pk_len)) {
    // If data read fails, handle the error (optional: resize back or clear)
    out_pk_val.clear();
    return TableFileDeserializationIndicator::IOERROR;
  }
  // calculate the offsets after we are certain that we managed to process a live record without IO errors
  out_offset += offset_delta;
  out_next_offset_start = record_length - offset_delta;
  return TableFileDeserializationIndicator::LIVE;
}
// table header was already read by this point, so the seekg pointer is 32 bytes ahead of file.beg
/**
 * @param number_of_columns_without_pk Should already be adjusted to exclude the primary key -
 * it is used to adjust the seekg pointer to skip the column offset region
 * @param table_file read only binary stream
 */
index_ptr_t FileHandler::extractPrimaryKeysIndex(std::ifstream& table_file, const size_t number_of_columns_without_pk) {
  auto result = std::make_unique<index_t>();
  std::string pk_val;
  std::uint64_t offset = 0ul;
  std::uint64_t next_offset_start = 0ul;
  auto serialization_indicator_result = FileHandler::deserializeNextPrimaryKey(
      table_file, pk_val, offset, next_offset_start, number_of_columns_without_pk);
  // iterate through every record in the file and read the primary key
  while (serialization_indicator_result != TableFileDeserializationIndicator::ENDOFTABLE) {
    result->insert(std::pair<std::string, std::uint64_t>(pk_val, offset));
    offset += next_offset_start;
    serialization_indicator_result = FileHandler::deserializeNextPrimaryKey(
        table_file, pk_val, offset, next_offset_start, number_of_columns_without_pk);
  }

  return std::move(result);
}