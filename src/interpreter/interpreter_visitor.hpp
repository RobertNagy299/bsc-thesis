#pragma once
#include "execution_context.hpp"
#include "../parser/ast.hpp"
#include <iostream>

struct InterpreterVisitor : ASTVisitor
{
private:
  ExecutionContext &ctx;

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
    // Store schema in context
    std::vector<UntypedColumnDefNode *> schema;
    for (auto c : node.columns)
    {
      auto col = dynamic_cast<UntypedColumnDefNode *>(c);
      if (col)
        schema.push_back(col);
    }
    ctx.untyped_tables[node.tableName] = schema;
    std::cout << "Created table " << node.tableName
              << " with " << schema.size() << " columns\n";
  }

  void visit(UntypedColumnDefNode &node) override
  {
    // Usually handled by parent (CreateUntypedTableNode)
    std::cout << "Column: " << node.name << "\n";
  }
};
