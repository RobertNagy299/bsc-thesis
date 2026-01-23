#include "../public_api.hpp"

void FileHandler::updateData(const UpdateNode& node, ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Update' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);
  // Update is supposed to mark the old record as a tombstone, and insert a new record with
  // the new (and old) data.
  // open the file in input/output mode because we need to read the record and also overwrite record types
  std::fstream table_file(table_file_path, std::ios::in | std::ios::out | std::ios::binary);
  FileHandler::checkFileValidity(table_file, node.table_name);

  // 1.: Extract the old records and mark them as deleted
  std::vector<std::unique_ptr<DB_Types::Record>>
      old_records; // store records on heap, because they can be big - might not fit on stack
  const auto& schema = ctx.getUntypedTables().at(node.table_name);
  DB_Types::TableFileDeserializationIndicator record_deser_result;
  DB_Types::Record record_buffer;

  do {
    record_deser_result = FileHandler::Deserializer::deserializeNextRecordAndMarkAsTombstone(
        table_file, schema, node.opt_where_node, record_buffer);
    if (record_deser_result == DB_Types::TableFileDeserializationIndicator::LIVE) {
      old_records.push_back(std::make_unique<DB_Types::Record>(std::move(record_buffer)));
    }
    if (record_deser_result == DB_Types::TableFileDeserializationIndicator::IOERROR) {
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::NOCONTX_FILEOPS_UnknownIoErrorDuringDeserialization);
    }
  } while (record_deser_result != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE);

  // 2.: construct the new records based on the assignment list
  std::vector<std::unique_ptr<ValueRecordNode>> new_records;
  for (const auto& old_record : old_records) {
    std::vector<std::unique_ptr<LiteralNode>> new_values;
    for (std::size_t i = 0; i < schema.size(); ++i) {
      if (node.inverse_proj_mask[i]) {
        // old value is needed
        new_values.emplace_back(std::make_unique<LiteralNode>(LiteralNode::Type::STRING, old_record->at(i)));
      } else {
        // new value is needed
        new_values.emplace_back(
            std::make_unique<LiteralNode>(LiteralNode::Type::STRING, node.schema_index_to_literal_map.at(i)));
      }
    }
    new_records.push_back(std::make_unique<ValueRecordNode>(std::move(new_values)));
  }

  // 3. Append the updated records to the table file
  std::vector<bool> write_projection_mask;
  write_projection_mask.assign(schema.size(), true);
  std::ofstream append_table_file(table_file_path, std::ios::app | std::ios::binary);
  if (append_table_file.is_open()) {
    for (const auto& new_record : new_records) {
      FileHandler::Serializer::serializeNormalizedRecord(ctx, node.table_name, new_record.get(), write_projection_mask,
                                                         append_table_file, DB_Types::RecordType::UPDATE, true);
    }
  }
  // 4. close the files
  if (append_table_file.is_open()) { append_table_file.close(); }
  if (table_file.is_open()) { table_file.close(); }

  // 5. perform compaction due to tombstones
  double tombstone_ratio = FileHandler::Compactor::calculateTombstoneRatio(node.table_name);
  LoggerService::StatusLogger::printAsStandardOutput("Tombstone ratio = " + std::to_string(tombstone_ratio));
  if (tombstone_ratio >= FileHandler::Compactor::COMPACTION_THRESHOLD) {
    LoggerService::StatusLogger::printAsStandardOutput(
        "Tombstone ratio is greater than or equal to the compaction treshold, which is " +
        std::to_string(FileHandler::Compactor::COMPACTION_THRESHOLD) + ". Performing compaction...");
    FileHandler::Compactor::compactTable(node.table_name, ctx);
  }

  // 6. invalidate indices and update them for this table
  ctx.recalculateIndicesForTable(node.table_name);

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Update finished in " + std::to_string(double_duration.count()) +
                                                     " ms");
}
