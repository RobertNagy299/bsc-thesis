#include "../public_api.hpp"

void FileHandler::Serializer::serializeNormalizedRecord(const ExecutionContext& ctx, const std::string& table_name,
                                                        const ValueRecordNode* const& record,
                                                        const std::vector<bool>& projection_mask,
                                                        std::ofstream& table_file, DB_Types::RecordType type,
                                                        bool persist_to_disk) {
  if (!persist_to_disk) return;

  const auto& table_cols = ctx.getUntypedTables().at(table_name);
  const auto& colcode_map = ctx.getTableColcodeMap().at(table_name);

  const std::size_t ncols = table_cols.size();

  // --- compute PK index ---
  std::uint8_t pk_idx = 0;
  for (; pk_idx < ncols; ++pk_idx)
    if (Utilities::InsertUtils::getModifiers(table_cols[pk_idx]->modifiers).primary_key) break;

  // --- PK payload ---
  const std::string pk_literal = Utilities::StringUtils::removeOuterQuotes(record->values[pk_idx]->value);
  std::cout << " Final primary key literal in normalized-record.cpp = " << pk_literal << '\n';
  std::uint64_t record_length = sizeof(DB_Types::RecordType);
  record_length += sizeof(std::uint64_t) + pk_literal.size();

  std::vector<DB_Types::column_offset_t> column_offsets;
  std::vector<std::string> payload_literals;

  // we don't store the offset for the primary key, hence we subtract 1
  column_offsets.reserve(ncols - 1);

  std::uint8_t offset_index = 1;
  std::uint8_t remaining_cols = ncols - 1;
  std::uint64_t running_data_offset = 0;

  for (std::size_t i = 0; i < ncols; ++i) {
    if (i == pk_idx) continue;

    const auto& col = table_cols[i];
    const auto modifiers = Utilities::InsertUtils::getModifiers(col->modifiers);

    std::string final_literal;

    const auto& lit = record->values[i];
    const bool user_provided = projection_mask[i];
    const bool is_empty = !lit || lit->type == LiteralNode::Type::EMPTY;

    // potentially erroneous logic
    if (!user_provided || is_empty) {
      // --- DEFAULT substituted here ---
      final_literal = Utilities::InsertUtils::getDefaultValue(col->modifiers);
    } else {
      final_literal = Utilities::StringUtils::removeOuterQuotes(lit->value);
    }

    const std::uint8_t colcode = colcode_map->at(col->name);

    std::uint64_t remaining = std::max<std::uint64_t>(remaining_cols - offset_index, 0);

    std::uint64_t total_offset = remaining * sizeof(DB_Types::column_offset_t) + running_data_offset;
    // empty initialization to ensure initialized padding for the struct
    DB_Types::column_offset_t off{};
    off.offset = total_offset;
    off.col_id = colcode;
    column_offsets.push_back(off);

    payload_literals.push_back(final_literal);
    std::cout << " in normalized-record.cpp, i = " << std::to_string(i) << " and final literal = " << final_literal
              << '\n';
    running_data_offset += sizeof(std::uint64_t) + final_literal.size();
    offset_index++;
  }

  record_length += running_data_offset + remaining_cols * sizeof(DB_Types::column_offset_t);

  // ---- write ----
  try {
    FileHandler::writeToBinaryFile(table_file, record_length);
    FileHandler::writeToBinaryFile(table_file, type);

    for (auto& off : column_offsets) FileHandler::writeToBinaryFile(table_file, off);

    FileHandler::writeToBinaryFile(table_file, pk_literal);

    for (auto& lit : payload_literals) FileHandler::writeToBinaryFile(table_file, lit);
  } catch (const std::exception& exc) {
    LoggerService::ErrorLogger::handleFatalError(StatusCode::FatalErrorCode::FILEOPS_GenericFileIOFailure,
                                                 std::vector<std::string>{exc.what()});
  }
}
