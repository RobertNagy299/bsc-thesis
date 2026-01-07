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

namespace SemanticValidator {
const bool validateWhereClauseSemantics(const DB_Types::untyped_table_t::const_iterator& table, WhereNode* node);

const bool validateInsertSemantics(InsertNode& node, const ExecutionContext& ctx);
const bool validateSelectSemantics(SelectNode& node, const ExecutionContext& ctx);
const bool validateDeleteSemantics(DeleteNode& node, const ExecutionContext& ctx);
const bool validateUpdateSemantics(UpdateNode& node, const ExecutionContext& ctx);
}; // namespace SemanticValidator