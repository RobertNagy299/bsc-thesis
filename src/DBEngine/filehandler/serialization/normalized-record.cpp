#include "../public_api.hpp"

void FileHandler::Serializer::serializeNormalizedRecord(ExecutionContext& ctx, const std::string& table_name,
                                                        const ValueRecordNode* const& record,
                                                        const std::vector<bool>& projection_mask,
                                                        std::ofstream& table_file, DB_Types::RecordType type,
                                                        bool persist_to_disk) {
  if (!persist_to_disk) return;

  const auto& table_cols = ctx.getUntypedTables().at(table_name);
  const auto& colcode_map = ctx.getTableColcodeMap().at(table_name);

  const std::size_t ncols = table_cols.size();

  // --- compute PK's position index in the canonical schema ---
  std::uint8_t pk_idx = 0;
  for (; pk_idx < ncols; ++pk_idx)
    if (Utilities::InsertUtils::getModifiers(table_cols[pk_idx]->modifiers).primary_key) break;

  // --- PK payload ---
  const std::string pk_literal = Utilities::StringUtils::removeOuterQuotes(record->values[pk_idx]->value);
  std::uint64_t record_length = sizeof(DB_Types::RecordType);
  record_length += sizeof(std::uint64_t) + pk_literal.size();

  std::vector<DB_Types::column_offset_t> column_offsets;
  std::vector<std::string> payload_literals;

  // we don't store the offset for the primary key, hence we subtract 1
  column_offsets.reserve(ncols - 1);

  std::uint8_t offset_index = 1;
  std::uint8_t number_of_cols_without_pk = ncols - 1;
  std::uint64_t running_data_offset = 0;

  for (std::size_t i = 0; i < ncols; ++i) {
    if (i == pk_idx) continue;

    const auto& col = table_cols[i];
    const auto modifiers = Utilities::InsertUtils::getModifiers(col->modifiers);

    std::string final_literal; // will be initialized based on certain conditions

    const auto& lit = record->values[i];
    const bool user_provided = projection_mask[i];
    const bool is_empty = !lit || lit->type == LiteralNode::Type::EMPTY;

    // final_literal is initialized here
    if (!user_provided || is_empty) {
      // --- DEFAULT substituted here ---
      final_literal = Utilities::InsertUtils::getDefaultValue(col->modifiers);
    } else {
      final_literal = Utilities::StringUtils::removeOuterQuotes(lit->value);
    }

    const std::uint8_t colcode = colcode_map->at(col->name);

    // empty initialization to ensure initialized padding for the struct
    DB_Types::column_offset_t off{};
    // offset should mean: distance from the beginning of the data_tuple region to the start of the column's [size]
    // region
    off.offset = running_data_offset;
    off.col_id = colcode;
    column_offsets.push_back(off);

    payload_literals.push_back(final_literal);

    running_data_offset += sizeof(std::uint64_t) + final_literal.size();
    offset_index++;
  }

  record_length += running_data_offset + number_of_cols_without_pk * sizeof(DB_Types::column_offset_t);

  // ---- write ----
  try {
    FileHandler::writeToBinaryFile(table_file, record_length);
    FileHandler::writeToBinaryFile(table_file, type);
    // add the primary key to the index here in the case of INSERT or UPDATE
    if (type == DB_Types::RecordType::INSERT || type == DB_Types::RecordType::UPDATE) {
      ctx.upsertPrimaryKeyIndex(table_name, table_cols.at(pk_idx)->name, pk_literal, table_file.tellp());
    }

    for (auto& off : column_offsets) FileHandler::writeToBinaryFile(table_file, off);

    FileHandler::writeToBinaryFile(table_file, pk_literal);

    for (auto& lit : payload_literals) FileHandler::writeToBinaryFile(table_file, lit);
  } catch (const std::exception& exc) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::FILEOPS_GenericFileIOFailure,
                                                 std::vector<std::string>{exc.what()});
  }
}
