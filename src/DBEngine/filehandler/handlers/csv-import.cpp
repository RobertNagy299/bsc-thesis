#include "../public_api.hpp"
#include <sstream>

void FileHandler::CSVImporter::performCSVImport(const CSVImportNode& node, ExecutionContext& ctx) {
  const std::string& table_name = node.table_name;
  const std::string& csv_file_path = Utilities::StringUtils::removeOuterQuotes(node.file_path);

  FileHandler::ensureTableFileExists(table_name);
  const std::string& table_path = FileHandler::getTableFilePath(table_name);
  const std::size_t ncols = ctx.getUntypedTables().at(table_name).size();
  std::ifstream csv_file(csv_file_path);
  if (!csv_file.is_open()) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::NOCONTX_CSV_IMPORT_CannotOpenFile);
  }
  std::ofstream table_file(table_path, std::ios::binary | std::ios::app);

  std::string line;
  size_t line_count = 0;
  std::vector<bool> artificial_projection_mask(ncols, true);
  try {
    while (std::getline(csv_file, line)) {
      auto values = FileHandler::CSVImporter::parseCsvRow(line); // vector<string>
      ++line_count;
      if (values.size() != ncols) {
        // if we find a bad row, just skip it, and keep inserting values.
        LoggerService::WarningLogger::printAsStandardOutput(
            StatusCode::WarningCode::CSV_IMPORT_RowHasMoreColumnsThanTable,
            std::vector<std::string>{std::to_string(values.size()), std::to_string(line_count), table_name,
                                     std::to_string(ncols)});
      };
      auto record = ValueRecordNode(values);
      FileHandler::Serializer::serializeNormalizedRecord(ctx, table_name, &record, artificial_projection_mask,
                                                         table_file, DB_Types::RecordType::INSERT, true);
    }

    if (csv_file.is_open()) { csv_file.close(); }
    if (table_file.is_open()) { table_file.close(); }
  } catch (std::exception& e) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::CSV_IMPORT_UnknownException,
                                                 std::vector<std::string>{e.what()});
    if (csv_file.is_open()) { csv_file.close(); }
    if (table_file.is_open()) { table_file.close(); }
  }
}

std::vector<std::string> FileHandler::CSVImporter::parseCsvRow(const std::string& row) {
  std::vector<std::string> result;
  std::string current;
  bool in_quotes = false;

  for (size_t i = 0; i < row.size(); ++i) {
    char c = row[i];

    if (c == '"') {
      in_quotes = !in_quotes; // Toggle state
                              // Optional: current += c; // Keep quotes if you want them in the result
    } else if (c == ',' && !in_quotes) {
      result.push_back(current);
      current.clear();
    } else {
      current += c;
    }
  }
  result.push_back(current); // Push the final field
  return result;
}
