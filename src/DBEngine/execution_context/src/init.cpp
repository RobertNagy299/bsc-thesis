#include "../public_api.hpp"

void ExecutionContext::init() {
  if (!std::filesystem::exists(metadata_base_dir)) {
    LoggerService::ErrorLogger::printAsStandardError("No schema metadata found at " + metadata_base_dir +
                                                     ".\n Error source: " + __FILE__ + " Line " +
                                                     std::to_string(__LINE__));
    return;
  }
  LoggerService::StatusLogger::printAsStandardOutput("Initializing Execution Context...\nReading table metadata...");
  for (auto& entry : std::filesystem::directory_iterator(metadata_base_dir)) {
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
      untyped_tables[tableName] = schema;
      LoggerService::StatusLogger::printAsStandardOutput("Recognized table " + tableName + " with " +
                                                         std::to_string(schema.size()) + " columns.");
    }
  }
}
