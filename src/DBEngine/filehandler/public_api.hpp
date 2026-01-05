#pragma once
#include "../../auxiliary/utils_public_api.hpp"
#include "../../parser/ast.hpp"
#include "../execution_context/public_api.hpp"
#include "../services/logger/public_api.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>

namespace FileHandler {
//  fields
inline const std::string SCHEMA_BASE_DIRECTORY = "../src/schema";
inline const std::string METADATA_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/metadata";
inline const std::string DATASTORAGE_BASE_DIRECTORY = FileHandler::SCHEMA_BASE_DIRECTORY + "/data";
inline const std::uint64_t DB_MAGIC = 0x4A5353514C7631L;
inline const std::uint64_t DB_VERSION = 0x01L;
inline const std::uint64_t DB_FLAGS = 0x0L;
inline const std::uint64_t DB_RESERVED = 0x0L;
// methods
void createUntypedTable(CreateUntypedTableNode& node, ExecutionContext& ctx);
void dropTable(const DropTableNode& node, ExecutionContext& ctx);
void insertData(InsertNode& node, const ExecutionContext& ctx);

void ensureTableFileExists(const std::string& table_name);
const std::string getTableFilePath(const std::string& table_name);
const std::string getTableFolderPath(const std::string& table_name);
DB_Types::index_ptr_t extractPrimaryKeysIndex(std::ifstream& table_file, const std::size_t number_of_columns);

template <typename T> void writeToBinaryFile(std::ofstream& outFile, const T& data) {
  outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
}
// Specialization for std::string to handle variable length strings correctly
void writeToBinaryFile(std::ofstream& outFile, const std::string& data);

namespace Serializer {
  void serializeNormalizedRecord(const ExecutionContext& ctx, const std::string& table_name,
                                 const ValueRecordNode* const& record, const std::vector<bool>& projection_mask,
                                 std::ofstream& table_file, DB_Types::RecordType type, bool persist_to_disk);

} // namespace Serializer
namespace Deserializer {
  DB_Types::TableFileDeserializationIndicator deserializeNextPrimaryKey(std::ifstream& ifs, std::string& out_pk_val,
                                                                        std::uint64_t& out_offset,
                                                                        const std::size_t number_of_columns);
} // namespace Deserializer

}; // namespace FileHandler
