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
    std::vector<UntypedColumnDefNode *> schema;
    for (auto c : node.columns)
    {
      auto col = dynamic_cast<UntypedColumnDefNode *>(c);
      if (col)
      {
        schema.push_back(col);
      }
    }
    ctx.untyped_tables[node.tableName] = schema;
    std::cout << "Created table " << node.tableName
              << " with " << schema.size() << " columns\n";
  }

  void visit(DropTableNode &node) override
  {
    std::cout << "Drop table command recognized!" << " Table name : " << node.tableName << "\n";
    // TODO(rnagy): implement table removal from file system and execution context
    // if (ctx.untyped_tables.find(node.tableName) != ctx.untyped_tables.end())
    // {
    //   std::string table_directory = this->base_directory + node.tableName;
    //   std::filesystem::remove_all(table_directory);

    //   return;
    // }
  }

  void visit(UntypedColumnDefNode &node) override
  {
    // Usually handled by parent (CreateUntypedTableNode)
    std::cout << "Column: " << node.name << "\n";
  }
};
