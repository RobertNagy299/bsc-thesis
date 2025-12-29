#pragma once
#include "../../parser/ast.hpp"
#include "../execution_context/public_api.hpp"
#include "../services/logger/public_api.hpp"
#include <chrono>
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
  static inline const std::string METADATA_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/metadata";
  static inline const std::string DATASTORAGE_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/data";

  // methods
  static void createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx);
  static void dropTable(DropTableNode& node, ExecutionContext& ctx);
  static void insertData(InsertNode& node, const ExecutionContext& ctx);

private:
  static inline const uint64_t DB_MAGIC = 0x4A5353514C7631L;
  static inline const uint64_t DB_VERSION = 0x01L;
  static inline const uint64_t DB_FLAGS = 0x0L;
  static inline const uint64_t DB_RESERVED = 0x0L;

  /**
   * Utils
   */
  static void ensureTableFileExists(const std::string& table_name);
  static const std::string getTableFilePath(const std::string& table_name);
  static const std::string getTableFolderPath(const std::string& table_name);

  template <typename T> static void writeToBinaryFile(std::ofstream& outFile, const T& payload);
  // Specialization for std::string to handle variable length strings correctly
  void writeToBinaryFile(std::ofstream& outFile, const std::string& data);
};
