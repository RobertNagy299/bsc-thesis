#pragma once
#include "../../auxiliary/utils_public_api.hpp"
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
  static inline const uint64_t DB_MAGIC = 0x4A5353514C7631L;
  static inline const uint64_t DB_VERSION = 0x01L;
  // methods
  static void createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx);
  static void dropTable(DropTableNode& node, ExecutionContext& ctx);
  static void insertData(InsertNode& node, const ExecutionContext& ctx);

  static void ensureTableFileExists(const std::string& table_name);
  static const std::string getTableFilePath(const std::string& table_name);
  static const std::string getTableFolderPath(const std::string& table_name);
  static index_ptr_t extractPrimaryKeysIndex(std::ifstream& table_file, const size_t number_of_columns);

private:
  static inline const uint64_t DB_FLAGS = 0x0L;
  static inline const uint64_t DB_RESERVED = 0x0L;

  /**
   * Utils
   */
  static TableFileDeserializationIndicator deserializeNextPrimaryKey(std::ifstream& ifs, std::string& out_pk_val,
                                                                     std::uint64_t& out_offset,
                                                                     std::uint64_t& out_next_offset_start,
                                                                     const size_t number_of_columns);
  template <typename T> static void writeToBinaryFile(std::ofstream& outFile, const T& data) {
    outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
  }
  // Specialization for std::string to handle variable length strings correctly
  static void writeToBinaryFile(std::ofstream& outFile, const std::string& data);
  static void serializeRecordWithoutColList(const ExecutionContext& ctx, const std::string& table_name,
                                            const ValueRecordNode* const& record, std::ofstream& table_file,
                                            const RecordType type, bool persist_to_disk);
};
