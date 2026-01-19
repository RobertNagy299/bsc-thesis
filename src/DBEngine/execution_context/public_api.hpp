#pragma once
#include "../../auxiliary/types/types.hpp"
#include "../../parser/ast.hpp"
#include "../services/logger/public_api.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
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

  DB_Types::untyped_table_t untyped_tables;
  DB_Types::table_colcode_map_t table_colcodes;
  DB_Types::indices_ptr_t indices = std::make_unique<DB_Types::tablename_idxmap_map_t>();

  static inline const std::string METADATA_BASE_DIR = "../src/schema/metadata";
  // TODO : implement thread safety (NVM: Implement GIL instead if you have time)
  mutable std::mutex m_mutex;
  static inline ExecutionContext* instance = nullptr;

  // private methods
  void initializeColumnEncodingMap();
  void initializeUntypedTableMetadata();
  void initializePrimaryKeyIndices();

public:
  void eraseKeyFromIndex(const std::string& table_name, const std::string& col_name, const std::string& key_value);
  void recalculateIndexForTable(const std::string& table_name);
  const DB_Types::indices_ptr_t& getHashmapIndices() const;
  const DB_Types::untyped_table_t& getUntypedTables() const;
  DB_Types::untyped_table_t& transferOwnershipOfUntypedTables();
  const DB_Types::table_colcode_map_t& getTableColcodeMap() const;
  void setUntypedTable(const std::string& table_name, const std::vector<UntypedColumnDefNode*>& coldefs);

  void eraseTable(const DB_Types::untyped_table_t::iterator&);
  void eraseTable(const std::string& table_name);

  void eraseInMemoryHashMapIndicesForTable(const std::string& table_name);

  static ExecutionContext& getInstance();
  static void destroyInstance();
};
