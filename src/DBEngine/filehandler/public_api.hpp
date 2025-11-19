#pragma once
#include "../../parser/ast.hpp"
#include "../execution_context/public_api.hpp"
#include "../services/logger/public_api.hpp"
#include <filesystem>
#include <fstream>

struct FileHandler {
  // Delete the default constructor to prevent instantiation
  FileHandler() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  FileHandler(const FileHandler&) = delete;
  FileHandler& operator=(const FileHandler&) = delete;

  // static fields
  static inline const std::string SCHEMA_BASE_DIRECTORY = "../src/schema";
  ;
  static inline const std::string METADATA_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/metadata";
  ;
  static inline const std::string DATASTORAGE_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/data";

  // methods
  static void createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx);
  static void dropTable(DropTableNode& node, ExecutionContext& ctx);
  static void insertData(InsertNode& node, ExecutionContext& ctx);
};
