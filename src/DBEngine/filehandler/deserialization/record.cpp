#include "../../services/condition-evaluator/public-api.hpp"
#include "../public_api.hpp"

DB_Types::TableFileDeserializationIndicator FileHandler::Deserializer::deserializeNextRecord(
    std::ifstream& table_file, const std::vector<UntypedColumnDefNode*>& schema,
    const std::vector<bool>& projection_mask, const WhereNode* where, DB_Types::Record& out_record) {

  std::uint64_t record_length;
  // 1. Attempt to read the length indicator
  if (!table_file.read(reinterpret_cast<char*>(&record_length), sizeof(record_length))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return DB_Types::TableFileDeserializationIndicator::ENDOFTABLE;
  }

  // read the record type
  DB_Types::RecordType record_type;
  if (!table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }

  // if it is a tombstone record, skip it
  if (record_type == DB_Types::RecordType::DELETE || record_type == DB_Types::RecordType::UNUSED) {
    // adjust the seekg pointer -- we are already 9 bytes into the record, and record length does not contain itself,
    // so we need to jump forward by [record_length - sizeof(record_type)] bytes
    table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
    return DB_Types::TableFileDeserializationIndicator::TOMBSTONE;
  }
  // skip the column offset region
  std::uint64_t number_of_columns_without_pk = schema.size() - 0x1ul;
  std::uint64_t col_offset_region_length = number_of_columns_without_pk * sizeof(DB_Types::column_offset_t);
  // at this point, this is a valid insert / update record. Let's skip through the col offset region
  table_file.seekg(col_offset_region_length, std::ios::cur);

  // At this point, the seekg pointer should be at the beginning of the primary key region
  // we can start extracting each value, then apply projection, etc.
  out_record.clear();
  out_record.reserve(schema.size());

  bool where_result = true;

  for (std::size_t i = 0; i < schema.size(); ++i) {

    // --- deserialize value (even if we don't project it) ---
    std::string deserialized_value;
    std::uint64_t value_size;

    table_file.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));
    deserialized_value.resize(value_size);

    // handle IO Error by clearing buffer
    if (!table_file.read(&deserialized_value[0], value_size)) {
      deserialized_value.clear();
      return DB_Types::TableFileDeserializationIndicator::IOERROR;
    }

    // --- WHERE evaluation (single condition only) ---
    if (where && where->conditions_list_node) {
      auto& cond = where->conditions_list_node->conditions[0];

      if (cond->schema_index == i) {
        // TODO comparator evaluation
        where_result =
            ConditionEvaluator::evaluateComparator(deserialized_value, cond->cmp_node->type, cond->literal_value);
      }
    }

    // --- projection ---
    if (projection_mask[i]) out_record.push_back(std::move(deserialized_value));
  }

  // signal to the executor that this record shouldn't be added to the final result set
  // because it doesn't meet the where condition
  if (!where_result) { return DB_Types::TableFileDeserializationIndicator::SKIP; }
  return DB_Types::TableFileDeserializationIndicator::LIVE;
}
