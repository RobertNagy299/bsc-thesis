#pragma once
#include "execution_context.hpp"
#include "../parser/ast.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

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
    std::cout << "Insertion visited" << '\n';
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
