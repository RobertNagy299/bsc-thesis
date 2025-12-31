#pragma once
#include "../../parser/ast.hpp"
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

typedef std::unordered_map<std::string, std::vector<UntypedColumnDefNode*>> untyped_table_t;
typedef std::unordered_map<std::string, uint8_t> colname_colcode_map_t;
typedef std::unordered_map<std::string, std::unique_ptr<colname_colcode_map_t>> table_colcode_map_t;

typedef struct ColModifierChecklist {
  bool has_default : 1;
  bool primary_key : 1;
  bool not_null : 1;
  bool unique : 1;
} colmodifiers_t;

/**
 * Index structure:
 * map<[table_name]-->map<[col_name]-->map<[literal]-->[offset]>>>
 */
// anonymous indices - only create indices for columns
// stores pairs of "primary key value" - "offset" or "unique value" - "offset" eg. "123 -> 432 bytes"
// offset is the absolute offset from the beginning of the record region to the start of the col offset region

// [literal] --> [offset]
typedef std::unordered_map<std::string, uint64_t> index_t;
typedef std::unique_ptr<index_t> index_ptr_t;

// [colname] --> map<[literal] --> [offset]>
typedef std::unordered_map<std::string, index_ptr_t> colname_literal_offset_map_t;
typedef std::unique_ptr<colname_literal_offset_map_t> colname_literal_offset_map_ptr_t;
// [table_name] --> map<[colname] --> map<[literal --> [offset]]>>
typedef std::unordered_map<std::string, colname_literal_offset_map_ptr_t> tablename_idxmap_map_t;
typedef std::unique_ptr<tablename_idxmap_map_t> indices_ptr_t;

enum class RecordType : uint8_t { INSERT, DELETE, UPDATE, UNUSED };
enum class TableFileDeserializationIndicator : uint8_t {
  ENDOFTABLE = 0xFFU,
  IOERROR = 0xFEU,
  TOMBSTONE = 0xFDU,
  LIVE = 0xFCU
};

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

// size is 16 bytes due to padding
typedef struct ColumnOffset {
  uint64_t offset;
  uint8_t col_id;
} column_offset_t;

// 8 bytes of length - used for file seeking
typedef struct RecordPayload {
  uint64_t record_len;
  RecordType record_type;
  std::vector<column_offset_t> column_offset_map; // vec.size = # of cols in table
  primary_key_t primary_key;                      // variable length, depends on the size of the key
  std::string payload;
} record_t;
