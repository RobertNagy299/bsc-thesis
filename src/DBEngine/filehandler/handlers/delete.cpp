#include "../../services/condition-evaluator/public-api.hpp"
#include "../public_api.hpp"

void FileHandler::deleteData(const DeleteNode& node, ExecutionContext& ctx) {
  LoggerService::StatusLogger::printAsStandardOutput("'Delete' command is valid - starting file operations...");
  auto start = std::chrono::steady_clock::now();

  FileHandler::ensureTableFileExists(node.table_name);
  const std::string table_file_path = FileHandler::getTableFilePath(node.table_name);

  // open the file in input/output mode because we need to read the record and also overwrite record types
  std::fstream table_file(table_file_path, std::ios::in | std::ios::out | std::ios::binary);
  FileHandler::checkFileValidity(table_file, node.table_name);

  auto table_cols = ctx.getUntypedTables().at(node.table_name);
  if (table_file.is_open()) {
    // perform actual tombstone logic
    // strategy - WHERE node contains a primaryKeyEQ comparison
    const std::string& primary_key_column = Utilities::ColumnUtils::extractPrimaryKeyColumn(table_cols);
    if (node.opt_where_node &&
        node.opt_where_node->conditions_list_node->conditions[0]->col_name == primary_key_column &&
        node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type == ComparatorNode::Type::EQ) {
      // perform index look-up for fast delete
      FileHandler::performDeleteByIndexLookup(node, ctx, table_file);
    } else {
      DB_Types::TableFileDeserializationIndicator delete_indicator;
      do {
        delete_indicator = FileHandler::performSequentialDelete(node, ctx, table_file);
      } while (delete_indicator != DB_Types::TableFileDeserializationIndicator::ENDOFTABLE);
      // invalidate indices for this table and recalculate them
      ctx.recalculateIndicesForTable(node.table_name);
    }
  }

  if (table_file.is_open()) { table_file.close(); }

  // check the tombstone ratio and perform compaction if necessary
  double tombstone_ratio = FileHandler::Compactor::calculateTombstoneRatio(node.table_name);
  LoggerService::StatusLogger::printAsStandardOutput("Tombstone ratio = " + std::to_string(tombstone_ratio));
  if (tombstone_ratio >= FileHandler::Compactor::COMPACTION_THRESHOLD) {
    LoggerService::StatusLogger::printAsStandardOutput(
        "Tombstone ratio is greater than or equal to the compaction treshold, which is " +
        std::to_string(FileHandler::Compactor::COMPACTION_THRESHOLD) + ". Performing compaction...");
    FileHandler::Compactor::compactTable(node.table_name, ctx);
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> double_duration = end - start;
  LoggerService::StatusLogger::printAsStandardOutput("Deletion finished in " + std::to_string(double_duration.count()) +
                                                     " ms");
}

/**
 * jumps to the specified record in the buffer based on the index
 * by the time this function is called, the table header should have been processed
 */
void FileHandler::performDeleteByIndexLookup(const DeleteNode& node, ExecutionContext& ctx, std::fstream& table_file) {
  // find the offset in the index
  const std::string& column_name = node.opt_where_node->conditions_list_node->conditions[0]->col_name;
  const std::string& key_value = Utilities::StringUtils::removeOuterQuotes(
      node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value);
  const auto& offset = ctx.getHashmapIndices()->at(node.table_name)->at(column_name)->at(key_value);
  // seek to the col offset region and then back to the col type region
  // offset is absolute, and seekp is not influenced by seekg, so we seek from the beg.
  table_file.seekp(offset, std::ios::beg);
  // overwrite the record type
  FileHandler::writeToBinaryFile(table_file, DB_Types::RecordType::DELETE);
  table_file.flush();
  // erase this key from the index
  ctx.eraseKeyFromIndex(node.table_name, column_name, key_value);
}

/**
 * Scans the file sequentially and deserializes the comparable literal.
 * This function is called after the file header was processed.
 * Warning - we static_cast the record length to a std::streamoff, which meant we lose a lot of potential values
 * because it is a downcast from unsigned long to regular long, but the compiler cries ambiguity if we don't do this
 */
DB_Types::TableFileDeserializationIndicator
FileHandler::performSequentialDelete(const DeleteNode& node, const ExecutionContext& ctx, std::fstream& table_file) {

  std::uint64_t record_length;
  // 1. Attempt to read the length indicator
  if (!table_file.read(reinterpret_cast<char*>(&record_length), sizeof(record_length))) {
    // If read fails (e.g., end of file reached or an error), return eof
    return DB_Types::TableFileDeserializationIndicator::ENDOFTABLE;
  }

  // mark the current spot for seekp if we need to delete this record
  const auto start_of_type_region = table_file.tellg();
  // read the record type
  DB_Types::RecordType record_type;
  if (!table_file.read(reinterpret_cast<char*>(&record_type), sizeof(record_type))) {
    return DB_Types::TableFileDeserializationIndicator::IOERROR;
  }

  // if it is a tombstone record, skip it
  if (record_type == DB_Types::RecordType::DELETE || record_type == DB_Types::RecordType::UNUSED) {
    // adjust the get and put pointers -- we are already 9 bytes into the record, and record length does not contain
    // itself, so we need to jump forward by [record_length - sizeof(record_type)] bytes
    table_file.seekg(record_length - sizeof(DB_Types::RecordType), std::ios::cur);
    return DB_Types::TableFileDeserializationIndicator::TOMBSTONE;
  }

  // at this point this is a live record. If there is no WHERE clause, we have to delete it.
  if (!node.opt_where_node) {
    table_file.seekp(start_of_type_region, std::ios::beg);
    FileHandler::writeToBinaryFile(table_file, DB_Types::RecordType::DELETE);
    // synchronize the buffers and move to the next record
    table_file.flush();
    table_file.seekg(start_of_type_region + static_cast<std::streamoff>(record_length), std::ios::beg);
    // indicate that a live record was found and deleted
    return DB_Types::TableFileDeserializationIndicator::LIVE;
  }

  // at this point, we know there is a WHERE condition and we need to evaluate it.
  // To do that, we need to deserialize the appropriate field.
  const std::string& comparator_colname = node.opt_where_node->conditions_list_node->conditions[0]->col_name;
  const std::string& comparator_value = Utilities::StringUtils::removeOuterQuotes(
      node.opt_where_node->conditions_list_node->conditions[0]->literal_value->value);
  // if we have a primary key comparison, we can skip the col offset region
  const auto& schema = ctx.getUntypedTables().at(node.table_name);
  const std::string& primary_key_colname = Utilities::ColumnUtils::extractPrimaryKeyColumn(schema);
  const auto number_of_cols_without_pk = schema.size() - 1ul;

  if (primary_key_colname == comparator_colname) {
    // in this case we have a PK comparison that's not EQ
    // We need to skip the coloffset_region and deserialize the primary key and evaluate the where condition
    table_file.seekg(sizeof(DB_Types::column_offset_t) * number_of_cols_without_pk, std::ios::cur);
    std::uint64_t pk_size;
    std::string pk_literal;
    table_file.read(reinterpret_cast<char*>(&pk_size), sizeof(pk_size));
    pk_literal.resize(pk_size);
    table_file.read(&pk_literal[0], pk_size);
    if (ConditionEvaluator::evaluateComparator(
            pk_literal, node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type,
            node.opt_where_node->conditions_list_node->conditions[0]->literal_value)) {
      // if we have a match, we have to delete the record
      table_file.seekp(start_of_type_region, std::ios::beg);
      FileHandler::writeToBinaryFile(table_file, DB_Types::RecordType::DELETE);
      // sync buffers and move to the next record
      table_file.flush();
      table_file.seekg(start_of_type_region + static_cast<std::streamoff>(record_length), std::ios::beg);
      return DB_Types::TableFileDeserializationIndicator::LIVE;
    }
  } else {
    // if our condition is not checking against a primary key, we have to deserialize the col offset region, find
    // which column we're dealing with, move the seekg pointer there and deserialize it, then check the condition
    const auto& colcode_of_comp_col = ctx.getTableColcodeMap().at(node.table_name)->at(comparator_colname);
    std::uint64_t final_offset = UINT64_MAX;
    for (std::size_t i = 0; i < number_of_cols_without_pk; ++i) {
      DB_Types::column_offset_t entry;
      table_file.read(reinterpret_cast<char*>(&entry), sizeof(entry));
      if (entry.col_id == colcode_of_comp_col) { final_offset = entry.offset; }
    }
    if (final_offset == UINT64_MAX) {
      if (table_file.is_open()) { table_file.close(); }
      LoggerService::ErrorLogger::handleFatalError(
          StatusCode::FatalErrorCode::NOCONTX_FILEOPS_DELETE_ColumnNotFoundDueToCorruption);
    }
    // at this point we know the offset for the proper column and we need to skip the PK region
    std::uint64_t pk_size;
    table_file.read(reinterpret_cast<char*>(&pk_size), sizeof(pk_size));
    table_file.seekg(pk_size, std::ios::cur);
    // we are the beginning of the data_tuple region, we can jump to the record
    table_file.seekg(final_offset, std::ios::cur);
    std::uint64_t literal_size;
    std::string lhs_literal;
    table_file.read(reinterpret_cast<char*>(&literal_size), sizeof(literal_size));
    lhs_literal.resize(literal_size);
    table_file.read(&lhs_literal[0], literal_size);
    // prepare the get pointer to point at the start of the next record
    // currently, we have travelled: record type + col offset region + pk region + final offset
    // this is a complex calculation, so we just jump back to the start of the type region
    // and then skip the entire record
    table_file.seekg(start_of_type_region, std::ios::beg);
    table_file.seekg(record_length, std::ios::cur);
    if (ConditionEvaluator::evaluateComparator(
            lhs_literal, node.opt_where_node->conditions_list_node->conditions[0]->cmp_node->type,
            node.opt_where_node->conditions_list_node->conditions[0]->literal_value)) {
      // if we have a match, we need to delete the record
      table_file.seekp(start_of_type_region, std::ios::beg);
      FileHandler::writeToBinaryFile(table_file, DB_Types::RecordType::DELETE);
      // sync buffers and move to the next record
      table_file.flush();
      table_file.seekg(start_of_type_region + static_cast<std::streamoff>(record_length), std::ios::beg);
      return DB_Types::TableFileDeserializationIndicator::LIVE;
    }
  }

  // if we didn't find anything, just sync the buffers, skip the record and return skip
  table_file.flush();
  table_file.seekg(start_of_type_region + static_cast<std::streamoff>(record_length), std::ios::beg);
  return DB_Types::TableFileDeserializationIndicator::SKIP;
}
