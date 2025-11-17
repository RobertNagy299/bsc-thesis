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
      std::cerr << "Insert statement is semantically invalid! The engine will not perform any file operations." << std::endl;
      return;
    }

    // Command is semantically valid, perform file operations
    DBEngine::FileHandler::insertData(node, ctx);
  }

  void visit(SelectNode &node) override
  {
    // TODO validate semantics for WHERE clause
    if (!SemanticValidator::validateSelectSemantics(node, ctx))
    {
      std::cerr << "Select statement is semantically invalid! The engine will not perform any file operations." << std::endl;
      return;
    }
    std::cout << "Select node detected, tableName = " << node.tableName << '\n'
              << "col list = ";
    if (!node.columns)
    {
      std::cout << " * ";
      return;
    }
    for (auto col : node.columns->columns)
    {
      std ::cout << col;
    }
    if (node.opt_where_node)
    {
      for (auto &cond : node.opt_where_node->conditions_list_node->conditions)
      {
        if (cond)
        {
          std::cout << "WHERE clause with: col name = ";
          std::cout << cond->col_name;
          std::cout << " literal: " << cond->literal_value->value << std::endl;
        }
      }
    }
  }

  void visit(DeleteNode &node) override
  {
    std::cout << "Delete node visited" << '\n';
  }

  void visit(WhereNode &node) override
  {
    std::cout << "Where node visited" << '\n';
  }

  void visit(ConditionListNode &node) override
  {
    std::cout << "Condition list node visited" << '\n';
  }

  void visit(ConditionNode &node) override
  {
    std::cout << "Condition node visited" << '\n';
  }

  void visit(ColumnListNode &node) override
  {
    std::cout << "ColumnListNode visited" << '\n';
  }

  void visit(ValuesListNode &node) override
  {
    std::cout << "Values list node visited" << '\n';
  }

  void visit(ComparatorNode &node) override
  {
    std::cout << "Comparator node visited" << '\n';
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
