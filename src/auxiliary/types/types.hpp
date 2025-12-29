#pragma once
#include "../../parser/ast.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

typedef std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>> untyped_table_t;
typedef std::unordered_map<std::string, uint8_t> colname_colcode_map_t;
typedef std::unordered_map<std::string, std::unique_ptr<colname_colcode_map_t>> table_colcode_map_t;

typedef struct ColModifierChecklist {
  bool has_default : 1;
  bool primary_key : 1;
  bool not_null : 1;
} colmodifiers_t;

// anonymous indeces - only create indeces for columns
// stores pairs of "primary key value" - "offset" or "unique value" - "offset"
typedef std::unordered_map<std::string, uint64_t> index_t;
// stores "table name" - "indeces"
typedef std::unordered_map<std::string, std::vector<index_t>> indeces_t;

enum class RecordType : uint8_t { INSERT, DELETE, UPDATE, UNUSED };

typedef struct PrimaryKeyPayload {
  uint64_t pk_len;
  std::string pk_value;
} primary_key_t;

// 32 bytes-long file header for a table.dat file
typedef struct TableFileHeader {
  uint64_t magic;
  uint64_t version;
  uint64_t flags;
  uint64_t reserved;
} table_file_header_t;

typedef struct ColumnOffset {
  uint8_t col_id;
  uint64_t offset;
} column_offset_t;

// 8 bytes of length - used for file seeking
typedef struct RecordPayload {
  uint64_t record_len;
  RecordType record_type;
  std::vector<column_offset_t> column_offset_map; // vec.size = # of cols in table
  primary_key_t primary_key;                      // variable length, depends on the size of the key
  std::string payload;
} record_t;
