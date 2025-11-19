#pragma once
#include "../../../auxiliary/utils_public_api.hpp"
#include "../../../parser/ast.hpp"
#include "../../execution_context/public_api.hpp"
#include "../logger/public_api.hpp"
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

/**
 * @brief
 * This class is intended to be a `static` method container, do not instantiate it!
 */
struct SemanticValidator {
  // Delete the default constructor to prevent instantiation
  SemanticValidator() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  SemanticValidator(const SemanticValidator&) = delete;
  SemanticValidator& operator=(const SemanticValidator&) = delete;

  static const bool validateInsertSemantics(InsertNode& node, ExecutionContext& ctx);
  static const bool validateSelectSemantics(SelectNode& node, ExecutionContext& ctx);
};