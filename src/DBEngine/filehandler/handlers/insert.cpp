#include "../public_api.hpp"

void FileHandler::insertData(InsertNode& node, const ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Insert' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.tableName);
  const std::string table_file_path = FileHandler::getTableFilePath(node.tableName);
  std::ofstream table_file(table_file_path, std::ios::app | std::ios::binary);

  const std::vector<ValueRecordNode*>& value_record_list = node.values->records;
  const std::size_t value_record_list_len = value_record_list.size();
  // command is valid, so we must check for missing literals and get their default values
  // 2 distinct cases: 1. the column list is specified. 2. it is not.
  // we need to iterate through each value record, calculate its length, and create the header
  if (!node.columns) {
    // TODO: filter for PK and unique violations inside semantic validator
    // TODO: Implement in-memory index in ctx.init();
    for (std::size_t i = 0; i < value_record_list_len; ++i) {
      // construct record header
      const auto& literal_values = node.values->records.at(i);
      if (table_file.is_open()) {
        FileHandler::serializeRecordWithoutColList(ctx, node.tableName, literal_values, table_file,
                                                   DB_Types::RecordType::INSERT, true);
      }
    }
  } else {
  }

  if (table_file.is_open()) { table_file.close(); }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Insertion finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
}
