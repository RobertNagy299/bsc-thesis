#pragma once
#include "../../../auxiliary/utils_public_api.hpp"
#include "../../../parser/ast.hpp"
#include "../../execution_context/public_api.hpp"
#include "../logger/public_api.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
/**
 * @brief
 * This class is intended to be a `static` method container, do not instantiate it!
 */
struct SemanticValidator {
private:
  static const bool validateWhereClauseSemantics(const untyped_table_t::const_iterator& table, WhereNode* node);
  // Delete the default constructor to prevent instantiation
  SemanticValidator() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  SemanticValidator(const SemanticValidator&) = delete;
  SemanticValidator& operator=(const SemanticValidator&) = delete;

public:
  static const bool validateInsertSemantics(InsertNode& node, const ExecutionContext& ctx);
  static const bool validateSelectSemantics(SelectNode& node, const ExecutionContext& ctx);
  static const bool validateDeleteSemantics(DeleteNode& node, const ExecutionContext& ctx);
  static const bool validateUpdateSemantics(UpdateNode& node, const ExecutionContext& ctx);
};