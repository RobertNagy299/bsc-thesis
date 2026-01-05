#include "../public_api.hpp"

void FileHandler::insertData(InsertNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Insert' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);
  std::ofstream table_file(table_file_path, std::ios::app | std::ios::binary);

  const std::vector<ValueRecordNode*>& value_record_list = node.values->records;
  const std::size_t value_record_list_len = value_record_list.size();
  // command is valid, and normalized, so just call the serializer
  for (const auto& record : node.values->records) {
    FileHandler::Serializer::serializeNormalizedRecord(ctx, node.table_name, record, node.projection_mask, table_file,
                                                       DB_Types::RecordType::INSERT, true);
  }

  if (table_file.is_open()) { table_file.close(); }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Insertion finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
}
