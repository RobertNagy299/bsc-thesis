#pragma once
#include "../../parser/ast.hpp"
#include "../services/logger/public_api.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Stateful Singleton
 */
struct ExecutionContext {
  std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>> untyped_tables;
  std::string metadata_base_dir = "../src/schema/metadata";

  void init();

  ~ExecutionContext();
};
