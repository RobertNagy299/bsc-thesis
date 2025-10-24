#pragma once
#include "execution_context.hpp"
#include "../parser/ast.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

std::string trim(const std::string &str);

struct InterpreterVisitor : ASTVisitor
{
private:
  ExecutionContext &ctx;
  std::string base_directory = "../src/schema/metadata";

public:
  InterpreterVisitor(ExecutionContext &c) : ctx(c) {}

  void visit(ProgramNode &node) override
  {
    for (auto stmt : node.statements)
    {
      stmt->accept(*this);
    }
  }

  void visit(CreateUntypedTableNode &node) override
  {
    if (ctx.untyped_tables.find(node.tableName) != ctx.untyped_tables.end())
    {
      std::cerr << "Error: Table " << node.tableName << " already exists.\n";
      return;
    }
    // Store metadata on disk
    // Base directory for schema
    std::filesystem::create_directories(this->base_directory); // ensures parents exist

    // Path for this table
    std::string table_path = this->base_directory + "/" + node.tableName;
    std::filesystem::create_directories(table_path);

    // Write metadata file
    std::string metadata_path = table_path + "/metadata.txt";
    std::ofstream file(metadata_path);
    if (!file.is_open())
    {
      std::cerr << "Error: Could not create metadata file for table "
                << node.tableName << "\n";
      return;
    }

    for (auto c : node.columns)
    {
      auto col = dynamic_cast<UntypedColumnDefNode *>(c);
      if (col)
      {
        file << col->name;
        if (!col->modifiers.empty())
          file << " ";
        for (size_t i = 0; i < col->modifiers.size(); ++i)
        {
          file << col->modifiers[i];
          if (i + 1 < col->modifiers.size())
            file << ",";
        }
        file << "\n";
      }
    }
    file.close();

    // Store schema in context
    std::vector<UntypedColumnDefNode *> cols;
    cols.reserve(node.columns.size());
    for (auto *col : node.columns)
    {
      auto casted_col = dynamic_cast<UntypedColumnDefNode *>(col);
      if (casted_col)
      {
        cols.push_back(new UntypedColumnDefNode(*casted_col)); // deep copy
      }
    }
    ctx.untyped_tables[node.tableName] = std::move(cols);

    std::cout << "Created table " << node.tableName
              << " with " << cols.size() << " columns\n";
  }

  void visit(DropTableNode &node) override
  {
    std::cout << "Drop table command recognized! Table name: " << node.tableName << "\n";

    std::string table_directory = this->base_directory + "/" + node.tableName;

    if (!std::filesystem::exists(table_directory))
    {
      std::cerr << "Error: table \"" << node.tableName << "\" does not exist on disk.\n";
      return;
    }

    try
    {
      // Remove directory and all metadata
      std::filesystem::remove_all(table_directory);

      // Now remove from execution context if it exists
      auto it = ctx.untyped_tables.find(node.tableName);
      if (it != ctx.untyped_tables.end())
      {
        // free the schema memory before erasing
        for (auto col : it->second)
        {
          delete col;
        }
        ctx.untyped_tables.erase(it);
      }

      std::cout << "Table \"" << node.tableName << "\" dropped successfully.\n";
    }
    catch (const std::filesystem::filesystem_error &e)
    {
      std::cerr << "Filesystem error while dropping table: " << e.what() << "\n";
    }
  }

  void visit(InsertNode &node) override
  {
    const auto &current_table = ctx.untyped_tables.find(node.tableName);
    if (current_table == ctx.untyped_tables.end())
    {
      std::cerr << "Table " << node.tableName << " does not exist.";
      return;
    }

    std::cout << "Table exists" << '\n';

    // node.columns is optional - if does not exist, is nullptr
    if (!node.columns)
    {
      std::cout << "No column list provided\n";
      const std::vector<ValueRecordNode *> &value_list = node.values->records;
      const std::vector<UntypedColumnDefNode *> &table_columns = current_table->second;
      const size_t value_record_length = value_list.size();
      const size_t table_cols_length = table_columns.size();
      std::cout << "value record length: " << value_record_length << "\nFirst node length: " << value_list.at(0)->values.size()
                << '\n';
      // if there are more value nodes in a value record than in the table, throw an error
      if (value_record_length > table_cols_length)
      {
        std::cerr << "Error (code: INSRT-0001): " << node.tableName << " Only has " << table_cols_length << " columns, but you tried to insert"
                  << value_record_length << " values in a row.\n";
      }

      // if there are empty values, make sure the omited values either have a default value
      // or are nullable, otherwise throw an error.
      try
      {
        for (size_t i = 0; i < value_record_length; ++i)
        {
          const std::vector<LiteralNode *> &literal_nodes_list = value_list.at(i)->values;
          const size_t current_literal_values_length = literal_nodes_list.size();
          for (size_t j = 0; j < table_cols_length; ++j)
          {
            const std::vector<std::string> &current_modifiers = table_columns.at(j)->modifiers;
            if (j < current_literal_values_length)
            {
              if (literal_nodes_list.at(j)->type == LiteralNode::Type::EMPTY)
              {
                bool not_null = 0;
                bool primary_key = 0;
                bool has_default = 0;
                for (size_t k = 0; k < current_modifiers.size(); ++k)
                {
                  if (trim(current_modifiers.at(k)) == "NOT NULL")
                  {
                    not_null = 1;
                    std::cout << "found not null\n\n";
                  }
                }
                if (not_null)
                {
                  std::cerr << "Error (code: INSRT-0002) - cannot insert empty literal into column marked as NOT NULL\n";
                  return;
                }
              }
            }
          }
        }
      }
      catch (const std::exception &e)
      {
        std::cerr << "Error during value insertion: " << e.what() << '\n';
      }
    }
  }

  void visit(ColumnListNode &node) override
  {
    std::cout << "ColumnListNode visited" << '\n';
  }

  void visit(ValuesListNode &node) override
  {
    std::cout << "Values list node visited" << '\n';
  }

  void visit(LiteralNode &node) override
  {
    std::cout << "Literal node visited" << '\n';
  }

  void visit(ValueRecordNode &node) override
  {
    std::cout << "ValueRecordNode visited" << '\n';
  }

  void visit(UntypedColumnDefNode &node) override
  {
    // Usually handled by parent (CreateUntypedTableNode)
    std::cout << "Column: " << node.name << "\n";
  }
};

// Function to trim leading and trailing whitespace
std::string trim(const std::string &str)
{
  const std::string whitespace = " \t\n\r\f\v"; // Common whitespace characters

  // Find the first non-whitespace character
  size_t first_non_ws = str.find_first_not_of(whitespace);
  if (std::string::npos == first_non_ws)
  {
    return ""; // Entire string is whitespace
  }

  // Find the last non-whitespace character
  size_t last_non_ws = str.find_last_not_of(whitespace);

  // Extract the substring
  return str.substr(first_non_ws, (last_non_ws - first_non_ws + 1));
}
