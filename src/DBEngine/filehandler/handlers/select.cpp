#include "../public_api.hpp"

std::unique_ptr<DB_Types::ResultSet> FileHandler::selectData(const SelectNode& node, const ExecutionContext& ctx) {

  LoggerService::StatusLogger::printAsStandardOutput("'Select' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);

  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);
  std::ifstream table_file(table_file_path, std::ios::binary);
  auto current_table = ctx.getUntypedTables().at(node.table_name);
  auto results = std::make_unique<DB_Types::ResultSet>();

  if (table_file.is_open()) {
    FileHandler::checkFileValidity(table_file, node.table_name);
    // TODO implement special case indexing for "primaryKeyEquality"

    // sequential scanning of the whole file
    DB_Types::Record record_buffer;
    auto deserialization_indicator = FileHandler::Deserializer::deserializeNextRecord(
        table_file, current_table, node.projection_mask, node.opt_where_node, record_buffer);

    while (deserialization_indicator != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE) {
      if (deserialization_indicator == DB_Types::TableFileDeserializationIndicator::LIVE) {
        results->push_back(std::move(record_buffer));
      }
      deserialization_indicator = FileHandler::Deserializer::deserializeNextRecord(
          table_file, current_table, node.projection_mask, node.opt_where_node, record_buffer);
    }
  }

  if (table_file.is_open()) { table_file.close(); }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Selection finished in " +
                                                     std::to_string(double_duration.count()) + " ms");
  return results;
}
