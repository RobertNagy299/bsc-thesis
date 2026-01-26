#include "../public_api.hpp"
#include <iomanip>

// Utils
namespace {
void printSeparator(const std::vector<std::size_t>& widths) {
  std::cout << '+';
  for (auto w : widths) { std::cout << std::string(w, '-') << '+'; }
  std::cout << '\n';
}

void printRow(const std::vector<std::string>& row, const std::vector<std::size_t>& widths) {
  std::cout << '|';
  for (std::size_t i = 0; i < row.size(); ++i) {
    std::cout << ' ' << std::left << std::setw(widths[i] - 1) << (row[i].empty() ? "NULL" : row[i]) << '|';
  }
  std::cout << '\n';
}
} // namespace

// Main methods

void LoggerService::StatusLogger::printAsStandardOutput(const std::string& status_msg) {
  std::cout << status_msg << std::endl;
}

void LoggerService::StatusLogger::printResultSetAsTable(const SelectNode& node,
                                                        const std::unique_ptr<DB_Types::ResultSet>& results) {
  if (results->empty()) {
    std::cout << "(0 rows)\n";
    return;
  }

  // --- headers ---
  std::vector<std::string> headers;
  for (std::size_t i = 0; i < node.projection_mask.size(); ++i) {
    if (node.projection_mask[i]) { headers.push_back(node.columns->columns[i]); }
  }

  // --- column widths ---
  std::vector<std::size_t> col_widths(headers.size(), 0);

  for (std::size_t i = 0; i < headers.size(); ++i) col_widths[i] = headers[i].size();

  for (const auto& row : *results) {
    for (std::size_t i = 0; i < row.size(); ++i) { col_widths[i] = std::max(col_widths[i], row[i].size()); }
  }

  for (auto& w : col_widths) w += 2;

  // --- print ---
  printSeparator(col_widths);
  printRow(headers, col_widths);
  printSeparator(col_widths);

  for (const auto& row : *results) { printRow(row, col_widths); }

  printSeparator(col_widths);
  std::cout << results->size() << " row(s)\n";
}

void LoggerService::StatusLogger::printTableDescription(const DescribeNode& node,
                                                        const std::vector<UntypedColumnDefNode*> schema) {

  std::size_t max_coldef_length = 0;
  std::vector<std::string> coldefs_to_print;

  // determine the longest column def and construct the complete column definition
  for (const auto& col_def : schema) {
    std::string coldef_to_print = col_def->name;
    for (const std::string& modifier : col_def->modifiers) { coldef_to_print += " " + modifier; }
    max_coldef_length = std::max(max_coldef_length, coldef_to_print.size());
    coldefs_to_print.push_back(coldef_to_print);
  }
  // print the column definitions in a table
  std::vector<std::size_t> col_widths(1, max_coldef_length + 2u);
  printSeparator(col_widths);
  for (const auto& coldef_to_print : coldefs_to_print) {
    printRow(std::vector<std::string>{coldef_to_print}, col_widths);
  }
  printSeparator(col_widths);
}
