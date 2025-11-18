#pragma once
#include "../DBEngine/filehandler/public_api.hpp"
#include "../DBEngine/services/logger/public_api.hpp"
#include "../DBEngine/services/semantic_validator/public_api.hpp"
#include "../auxiliary/utils_public_api.hpp"
#include "../parser/ast.hpp"
#include "execution_context.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// Interpreter
struct InterpreterVisitor : ASTVisitor {
private:
  ExecutionContext& ctx;

public:
  InterpreterVisitor(ExecutionContext& c) : ctx(c) {}

  void visit(ProgramNode& node) override {
    for (auto stmt : node.statements) { stmt->accept(*this); }
  }

  void visit(CreateUntypedTableNode& node) override { FileHandler::createUntypedTable(node, ctx); }

  void visit(DropTableNode& node) override { FileHandler::dropTable(node, ctx); }

  void visit(InsertNode& node) override {
    // Semantic validation - ensure proper literal values, types, etc.
    if (!SemanticValidator::validateInsertSemantics(node, ctx)) {
      LoggerService::ErrorLogger::printAsStandardError(
          "Insert statement is semantically invalid! The engine will not perform any file operations.");
      return;
    }

    // Command is semantically valid, perform file operations
    FileHandler::insertData(node, ctx);
  }

  void visit(SelectNode& node) override {
    // TODO validate semantics for WHERE clause
    if (!SemanticValidator::validateSelectSemantics(node, ctx)) {
      LoggerService::ErrorLogger::printAsStandardError(
          "Select statement is semantically invalid! The engine will not perform any file operations.");
      return;
    }
    std::cout << "Select node detected, tableName = " << node.tableName << '\n' << "col list = ";
    if (!node.columns) {
      std::cout << " * ";
      return;
    }
    for (auto col : node.columns->columns) { std ::cout << col; }
    if (node.opt_where_node) {}
  }

  void visit(UpdateNode& node) override { std::cout << "Update node visited" << '\n'; }

  void visit(AssignmentNode& node) override { std::cout << "assignment node visited" << '\n'; }

  void visit(AssignmentListNode& node) override { std::cout << "assignment list node visited" << '\n'; }

  void visit(DeleteNode& node) override { std::cout << "Delete node visited" << '\n'; }

  void visit(WhereNode& node) override { std::cout << "Where node visited" << '\n'; }

  void visit(ConditionListNode& node) override { std::cout << "Condition list node visited" << '\n'; }

  void visit(ConditionNode& node) override { std::cout << "Condition node visited" << '\n'; }

  void visit(ColumnListNode& node) override { std::cout << "ColumnListNode visited" << '\n'; }

  void visit(ValuesListNode& node) override { std::cout << "Values list node visited" << '\n'; }

  void visit(ComparatorNode& node) override { std::cout << "Comparator node visited" << '\n'; }

  void visit(LiteralNode& node) override { std::cout << "Literal node visited" << '\n'; }

  void visit(ValueRecordNode& node) override { std::cout << "ValueRecordNode visited" << '\n'; }

  void visit(UntypedColumnDefNode& node) override {
    // Usually handled by parent (CreateUntypedTableNode)
    std::cout << "Column: " << node.name << "\n";
  }
};
