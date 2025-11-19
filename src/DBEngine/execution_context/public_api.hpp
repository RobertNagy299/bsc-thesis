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
  std::string metadata_base_dir = "../src/schema/metadata";
  // TODO : implement thread safety
  mutable std::mutex m_mutex;
  static inline ExecutionContext* instance = nullptr;

public:
  const untyped_table_t& getUntypedTables() const;
  void setUntypedTable(const std::string& table_name, const std::vector<UntypedColumnDefNode*>& coldefs);

  void eraseTable(const untyped_table_t::iterator&);

  static ExecutionContext& getInstance();
  static void destroyInstance();
};
