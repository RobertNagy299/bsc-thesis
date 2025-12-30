#pragma once
#include "../../auxiliary/types/types.hpp"
#include "../../parser/ast.hpp"
#include "../services/logger/public_api.hpp"
#include <filesystem>
#include <fstream>
#include <mutex> // For thread-safe implementation
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Stateful Singleton
 */
struct ExecutionContext {

private:
  ExecutionContext();

  ExecutionContext(ExecutionContext&) = delete;
  ExecutionContext& operator=(ExecutionContext&) = delete;

  ~ExecutionContext();

  untyped_table_t untyped_tables;
  table_colcode_map_t table_colcodes;
  static inline const std::string METADATA_BASE_DIR = "../src/schema/metadata";
  // TODO : implement thread safety (NVM: Implement GIL instead if you have time)
  mutable std::mutex m_mutex;
  static inline ExecutionContext* instance = nullptr;

  // private methods
  void initializeColumnEncodingMap();
  void initializeUntypedTableMetadata();
  void initializePrimaryKeyIndeces();

public:
  const untyped_table_t& getUntypedTables() const;
  const table_colcode_map_t& getTableColcodeMap() const;
  void setUntypedTable(const std::string& table_name, const std::vector<UntypedColumnDefNode*>& coldefs);

  void eraseTable(const untyped_table_t::iterator&);

  static ExecutionContext& getInstance();
  static void destroyInstance();
};
