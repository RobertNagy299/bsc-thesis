#include "../../filehandler/public_api.hpp"
#include "../public_api.hpp"

void ExecutionContext::initializeUntypedTableMetadata() {
  if (!std::filesystem::exists(ExecutionContext::METADATA_BASE_DIR)) {
    LoggerService::ErrorLogger::printAsStandardError(
        "No schema metadata found at " + ExecutionContext::METADATA_BASE_DIR + ".\n Error source: " + __FILE__ +
        " Line " + std::to_string(__LINE__));
    return;
  }
  LoggerService::StatusLogger::printAsStandardOutput("Reading schema metadata...");
  for (auto& entry : std::filesystem::directory_iterator(ExecutionContext::METADATA_BASE_DIR)) {
    if (entry.is_directory()) {
      std::string tableName = entry.path().filename().string();
      std::string metadata_path = entry.path().string() + "/metadata.txt";

      if (!std::filesystem::exists(metadata_path)) continue;

      std::ifstream file(metadata_path);
      if (!file.is_open()) continue;

      std::vector<UntypedColumnDefNode*> schema;
      std::string line;
      while (std::getline(file, line)) {
        // Example format: "id PRIMARY KEY,NOT NULL"
        std::istringstream iss(line);
        std::string colName;
        iss >> colName;

        std::string modifiersPart;
        std::getline(iss, modifiersPart); // rest of line
        std::vector<std::string> modifiers;

        if (!modifiersPart.empty()) {
          std::stringstream ss(modifiersPart);
          std::string mod;
          while (std::getline(ss, mod, ',')) {
            if (!mod.empty()) modifiers.push_back(mod);
          }
        }

        schema.push_back(new UntypedColumnDefNode{colName, modifiers});
      }

      file.close();
      this->untyped_tables[tableName] = schema;
      LoggerService::StatusLogger::printAsStandardOutput("Recognized table " + tableName + " with " +
                                                         std::to_string(schema.size()) + " columns.");
    }
  }
}

void ExecutionContext::initializeColumnEncodingMap() {
  LoggerService::StatusLogger::printAsStandardOutput(
      "Constructing Column name - uint8_t encoding for binary file handling...");
  for (const auto& table : this->untyped_tables) {
    const std::string& table_name = table.first;
    const auto& table_cols = table.second;
    uint8_t col_code = (uint8_t)0x0u;
    auto map_ptr = std::make_unique<colname_colcode_map_t>();
    for (const auto& col : table_cols) {
      (*map_ptr)[col->name] = col_code;
      col_code += 0x1u;
      if (col_code == (uint8_t)0x0u) {
        LoggerService::ErrorLogger::printAsStandardError(
            "FATAL ERROR: (Code: OVRFLW-0001) Table " + table_name +
            " Has more than 256 columns, which is illegal. Terminating...");
        exit(1);
      }
    }
    // transfer ownership to member variable
    this->table_colcodes[table_name] = std::move(map_ptr);
  }
}

void ExecutionContext::initializePrimaryKeyIndeces() {
  // TODO;
  for (const auto& table : this->getUntypedTables()) {
    const std::string& table_name = table.first;
    FileHandler::ensureTableFileExists(table_name);
    const std::string& table_path = FileHandler::getTableFilePath(table_name);
    if (table_name != "users") { continue; }
    std::ifstream table_file(table_path, std::ios::binary | std::ios::in);
    if (table_file.is_open()) {
      table_file_header_t file_header;
      table_file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
      if (file_header.magic != FileHandler::DB_MAGIC) {
        LoggerService::ErrorLogger::printAsStandardError(
            "FATAL ERROR: (Code: FILE-OPS-0002) Unknown table file format.");
        exit(1);
      }
      if (file_header.version != FileHandler::DB_VERSION) {
        LoggerService::ErrorLogger::printAsStandardError(
            "Warning: (Code: FILE-OPS-0001W) Table file was created with a different version of the DB engine. Expect "
            "undefined behavior.");
      }
      uint64_t rec_len;
      RecordType rec_type;
      table_file.read(reinterpret_cast<char*>(&rec_len), sizeof(rec_len));
      table_file.read(reinterpret_cast<char*>(&rec_type), sizeof(rec_type));
      std::cout << "Record length = " + std::to_string(rec_len) << '\n';
      std::vector<column_offset_t> offsets;
      column_offset_t offset;
      column_offset_t offset2;

      table_file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
      offsets.push_back(offset);
      table_file.read(reinterpret_cast<char*>(&offset2), sizeof(offset2));
      offsets.push_back(offset2);
      uint64_t pk_len;
      table_file.read(reinterpret_cast<char*>(&pk_len), sizeof(pk_len));
      std::string pk_val;
      std::cout << "RETARD PK LENGTH = " + std::to_string(pk_len) << '\n';
      pk_val.resize(pk_len);
      table_file.read(&pk_val[0], pk_len);
      std::cout << "RETARD PK  = " + pk_val + '\n';

      uint64_t first_column_value_length;
      table_file.read(reinterpret_cast<char*>(&first_column_value_length), sizeof(first_column_value_length));
      std::string first_column_val;
      std::cout << "first column value length = " + std::to_string(first_column_value_length) << '\n';
      first_column_val.resize(first_column_value_length);
      table_file.read(&first_column_val[0], first_column_value_length);
      std::cout << "FIRST COLUMN VAL = " + first_column_val << '\n';
    }

    if (table_file.is_open()) { table_file.close(); }
  }
}