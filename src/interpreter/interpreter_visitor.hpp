#pragma once
#include "execution_context.hpp"
#include "../parser/ast.hpp"
#include "utils.hpp"
#include "validators.hpp"
#include "../DBEngine/engine.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

// Interpreter
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
    DBEngine::FileHandler::createUntypedTable(node, ctx);
  }

  void visit(DropTableNode &node) override
  {
    DBEngine::FileHandler::dropTable(node, ctx);
  }

  void visit(InsertNode &node) override
  {
    // Semantic validation - ensure proper literal values, types, etc.
    if (!SemanticValidator::validateInsertSemantics(node, ctx))
    {
      return;
    }

    // Command is semantically valid, perform file operations
    DBEngine::FileHandler::insertData(node, ctx);
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
