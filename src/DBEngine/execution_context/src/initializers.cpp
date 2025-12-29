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