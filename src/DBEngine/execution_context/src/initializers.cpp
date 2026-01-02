#include "../../filehandler/public_api.hpp"
#include "../public_api.hpp"

void ExecutionContext::initializeUntypedTableMetadata() {
  if (!std::filesystem::exists(ExecutionContext::METADATA_BASE_DIR)) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::METADAT_MetadataDirectoryDoesNotExist,
                                                 std::vector<std::string>{ExecutionContext::METADATA_BASE_DIR});
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
      "Constructing Column name - std::uint8_t encoding for binary file handling...");
  for (const auto& table : this->untyped_tables) {
    const std::string& table_name = table.first;
    const auto& table_cols = table.second;
    std::uint8_t col_code = (std::uint8_t)0x0u;
    auto map_ptr = std::make_unique<DB_Types::colname_colcode_map_t>();
    for (const auto& col : table_cols) {
      (*map_ptr)[col->name] = col_code;
      col_code += 0x1u;
      if (col_code == (std::uint8_t)0x0u) {
        LoggerService::ErrorLogger::handleFatalError(
            StatusCode::FatalErrorCode::FILEOPS_ColOffsetRegionHasMoreColsThanAllowed,
            std::vector<std::string>{table_name});
      }
    }
    // transfer ownership to member variable
    this->table_colcodes[table_name] = std::move(map_ptr);
  }
}

void ExecutionContext::initializePrimaryKeyIndices() {
  for (const auto& table : this->getUntypedTables()) {
    const std::string& table_name = table.first;
    FileHandler::ensureTableFileExists(table_name);
    const std::string& table_path = FileHandler::getTableFilePath(table_name);
    // insert the pointer to this table's column - indeces map
    if (this != nullptr && this->indices != nullptr) {
      this->indices->insert(std::pair<std::string, DB_Types::colname_literal_offset_map_ptr_t>(
          table_name, std::make_unique<DB_Types::colname_literal_offset_map_t>()));
    } else {
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure);
    }

    std::ifstream table_file(table_path, std::ios::binary | std::ios::in);
    // store the index as a "pk" -> "offset" pair where pk is the string representation of the pk
    // and the offset is the relative offset from the start of the record to the start of the data tuple region
    if (table_file.is_open()) {
      DB_Types::table_file_header_t file_header;
      table_file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
      if (file_header.magic != FileHandler::DB_MAGIC) {
        LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::FILEOPS_UnknownTableFileFormat,
                                                     std::vector<std::string>{table_name});
      }
      if (file_header.version != FileHandler::DB_VERSION) {
        LoggerService::WarningLogger::printAsStandardOutput(
            StatusCode::WarningCode::FILEOPS_FileWasMadeWithDifferentDBVersion, std::vector<std::string>{table_name});
      }
      // perform deserialization logic
      const std::string& pk_col_name = Utilities::ColumnUtils::extractPrimaryKeyColumn(table.second);
      if (this != nullptr && this->indices != nullptr && this->indices->at(table_name) != nullptr) {
        // subtract 1 from the col list size due to the exclusion of the PK from the col offset region
        this->indices->at(table_name)
            ->insert(std::pair<std::string, DB_Types::index_ptr_t>(
                pk_col_name, FileHandler::extractPrimaryKeysIndex(table_file, table.second.size() - 1UL)));
        // TODO remove debugging logs
        for (const auto& rec : *(this->indices->at(table_name)->at(pk_col_name))) {
          std::cout << " in index, pk val = " << rec.first << " Offset = " << std::to_string(rec.second) << '\n';
        }
      } else {
        LoggerService::ErrorLogger::handleFatalError(
            StatusCode::FatalErrorCode::NOCONTX_NULLPTR_InMemoryPrimaryKeyHashMapInitializationFailure);
      }
    }

    if (table_file.is_open()) { table_file.close(); }
  }
}