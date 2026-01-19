#pragma once
#include "../DBEngine/execution_context/public_api.hpp"
#include "../DBEngine/filehandler/public_api.hpp"
#include "../DBEngine/services/logger/public_api.hpp"
#include "../DBEngine/services/semantic_normalizer/public-api.hpp"
#include "../DBEngine/services/semantic_validator/public_api.hpp"
#include "../auxiliary/utils_public_api.hpp"
#include "../parser/ast.hpp"
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
    if (!SemanticNormalizer::normalizeInsert(node, ctx) || !SemanticValidator::validateInsertSemantics(node, ctx)) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::NOCONTX_SEMVAL_INSERT_GenericInvalidStatement);
      return;
    }

    // Command is semantically valid, perform file operations
    FileHandler::insertData(node, ctx);
  }

  void visit(SelectNode& node) override {

    // call it here because normalizeSelect modifies the input node and also returns true or false.
    // Thus it would create dead code by making the validatator impossible to run inside the if condition
    bool normalized_successfully = SemanticNormalizer::normalizeSelect(node, ctx);

    if (!SemanticValidator::validateSelectSemantics(node, ctx) || !normalized_successfully) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::NOCONTX_SEMVAL_SELECT_GenericInvalidStatement);
      return;
    }
    // handle file operations
    auto results = FileHandler::selectData(node, ctx);

    // handle console printing, or sending off to a network endpoint later
    LoggerService::StatusLogger::printResultSetAsTable(node, results);

    // end of results's lifespan
    /*
    std::cout << "Select node detected, table_name = " << node.table_name << '\n' << "col list = ";
    if (!node.columns) {
      std::cout << " * ";
      return;
    }
    for (auto col : node.columns->columns) { std ::cout << col << ", "; }
    std::cout << "\nprojection mask: ";
    for (auto bit : node.projection_mask) { std::cout << bit << ", "; }
    std::cout << "\nschema indexes in where: ";
    for (auto& condNode : node.opt_where_node->conditions_list_node->conditions) {
      std::cout << std::to_string(condNode->schema_index) << ", ";
    }
      */
  }

  void visit(UpdateNode& node) override {
    if (!SemanticValidator::validateUpdateSemantics(node, ctx)) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::NOCONTX_SEMVAL_UPDATE_GenericInvalidStatement);
      return;
    }
    // TODO file operations and testing
    std::cout << "Update node visited" << '\n';
  }

  void visit(AssignmentNode& node) override { std::cout << "assignment node visited" << '\n'; }

  void visit(AssignmentListNode& node) override { std::cout << "assignment list node visited" << '\n'; }

  void visit(DeleteNode& node) override {
    // Validate semantics

    bool is_normalized = SemanticNormalizer::normalizeDelete(node, ctx);

    if (!SemanticValidator::validateDeleteSemantics(node, ctx) || !is_normalized) {
      LoggerService::ErrorLogger::printAsStandardError(
          StatusCode::ErrorCode::NOCONTX_SEMVAL_DELETE_GenericInvalidStatement);
      return;
    }

    // TODO semantically valid, perform file ops.
  }

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
