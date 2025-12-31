#include "../public_api.hpp"

void FileHandler::serializeRecordWithoutColList(const ExecutionContext& ctx, const std::string& table_name,
                                                const ValueRecordNode* const& record, std::ofstream& table_file,
                                                const DB_Types::RecordType type, bool persist_to_disk) {

  if (!persist_to_disk) {
    // TODO
    return;
  }
  /**
   * Calculate the record's length
   */

  uint64_t record_length = sizeof(DB_Types::RecordType);
  const std::vector<UntypedColumnDefNode*>& current_table_columns = ctx.getUntypedTables().at(table_name);
  const size_t number_of_table_cols = current_table_columns.size();
  const auto& colcode_map = ctx.getTableColcodeMap().at(table_name);

  // calculate the primary key payload size

  // find which column is the primary key
  uint8_t pk_idx = 0u;
  for (const auto& col_node : current_table_columns) {
    if (Utilities::InsertUtils::getModifiers(col_node->modifiers).primary_key) { break; }
    pk_idx += 1u;
  }
  // PK was verified to be non-empty by the semantic validator
  const auto& pk_literal_str = record->values.at(pk_idx)->value;
  size_t pk_size = pk_literal_str.size();
  record_length += (sizeof(pk_size) + pk_size);

  // calculate the data payload length (without the PK)
  // while simultaneously calculating the col offset map with relative offsets
  std::vector<DB_Types::column_offset_t> column_offsets;
  std::vector<std::string> literals_to_be_written_to_disk;
  // subtract 1 because we don't store the primary key like this
  column_offsets.reserve(current_table_columns.size() - 1ul);
  uint8_t col_offset_idx = 1u;
  uint8_t col_offset_region_length = current_table_columns.size() - 1u;
  uint64_t actual_coldata_offset_from_end_of_colcode_region = 0ul;
  for (size_t i = 0; i < current_table_columns.size(); ++i) {
    if (i == pk_idx) { continue; }

    std::string final_literal;
    // right hand side contains implicit empty literals
    if (i >= record->values.size()) {
      // get default value for empty literal
      final_literal = Utilities::InsertUtils::getDefaultValue(current_table_columns.at(i)->modifiers);
      std::cout << "Empty implicit literal, final value = " << final_literal << '\n';
    } else {
      // left hand side (either explicit empty literal, or non-empty literal)
      // get default value for empty literal
      if (record->values.at(i)->type == LiteralNode::Type::EMPTY) {
        final_literal = Utilities::InsertUtils::getDefaultValue(current_table_columns.at(i)->modifiers);
        std::cout << "Found explicit empty literal, default final value = " << final_literal << '\n';
      } else {
        final_literal = Utilities::StringUtils::removeOuterQuotes(record->values.at(i)->value);
        std::cout << "Found actual literal, final value = " << final_literal << '\n';
      }
    }
    std::cout << "final literal = " << final_literal << '\n';
    // final literal is known, we can calculate offsets
    const std::string& current_colname = current_table_columns.at(i)->name;
    const uint8_t& colcode = colcode_map->at(current_colname);

    size_t remaining_col_offset_records = std::max(col_offset_region_length - col_offset_idx, 0);
    uint64_t total_offset = remaining_col_offset_records * sizeof(DB_Types::column_offset_t) +
                            actual_coldata_offset_from_end_of_colcode_region;
    DB_Types::column_offset_t col_offset_rec{total_offset, colcode};

    column_offsets.push_back(col_offset_rec);
    literals_to_be_written_to_disk.push_back(final_literal);
    // update the state
    col_offset_idx += 1u;
    size_t literal_data_size = final_literal.size();
    actual_coldata_offset_from_end_of_colcode_region += (literal_data_size + sizeof(literal_data_size));
  }
  record_length +=
      (actual_coldata_offset_from_end_of_colcode_region + col_offset_region_length * sizeof(DB_Types::column_offset_t));

  // critical region - file operations
  try {
    FileHandler::writeToBinaryFile(table_file, record_length);
    std::cout << "Record length (bytes) = " << std::to_string(record_length) << '\n';
    FileHandler::writeToBinaryFile(table_file, type);
    for (const auto& offset_record : column_offsets) { FileHandler::writeToBinaryFile(table_file, offset_record); }
    FileHandler::writeToBinaryFile(table_file, pk_literal_str);
    for (const std::string& literal_payload : literals_to_be_written_to_disk) {
      std::cout << "Literal payload = " << literal_payload
                << " With size (Bytes) = " << std::to_string(literal_payload.size()) << '\n';
      FileHandler::writeToBinaryFile(table_file, literal_payload);
    }

  } catch (const std::exception& exception) {
    LoggerService::ErrorLogger::printAsStandardError(
        "FATAL ERROR (Code: FILE-OPS-0001): " + std::string(exception.what()) + " Coming from " + __FILE__ + " Line " +
        std::to_string(__LINE__));
    exit(1);
  }
}