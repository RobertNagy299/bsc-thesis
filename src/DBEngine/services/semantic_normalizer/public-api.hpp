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

namespace SemanticNormalizer {
bool normalizeInsert(InsertNode& node, const ExecutionContext& ctx);
}